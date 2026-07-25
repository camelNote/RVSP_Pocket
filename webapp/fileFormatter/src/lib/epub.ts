import JSZip from 'jszip'
import * as cheerio from 'cheerio'

type ZipEntry = {
  name: string
  directory: boolean
}

type ZipObjectLike = NonNullable<ReturnType<JSZip['file']>>

export type EpubTextResult = {
  chapterCount: number
  fileCount: number
  bookTitle: string
}

export type EpubTextChunk = {
  chapterIndex: number
  chapterTitle: string
  text: string
}

export type EpubTextProgress = {
  chapterIndex: number
  totalCandidates: number
  chapterTitle: string
  fileName: string
}

export type EpubTextOptions = {
  onChunk?: (chunk: EpubTextChunk) => void | Promise<void>
  onProgress?: (progress: EpubTextProgress) => void | Promise<void>
}

function normalizeWhitespace(value: string) {
  return value
    .replace(/\u00a0/g, ' ')
    .replace(/[\t\r\n]+/g, ' ')
    .replace(/\s{2,}/g, ' ')
    .trim()
}

function stripHtmlToText(html: string) {
  const $ = cheerio.load(html)

  $('script, style, img, svg, picture, source, noscript, nav, iframe, canvas, video, audio, button').remove()

  const title = normalizeWhitespace($('title').first().text())
  const heading = normalizeWhitespace($('h1, h2, h3, h4, h5, h6').first().text())
  const bodyText = normalizeWhitespace($('body').text() || $.root().text())

  if (!bodyText) {
    return { title, bodyText: '' }
  }

  return {
    title: title || heading,
    bodyText
  }
}

function resolveRelativePath(basePath: string, relativePath: string) {
  const baseParts = basePath.split('/').filter(Boolean)
  baseParts.pop()

  for (const part of relativePath.split('/')) {
    if (!part || part === '.') continue
    if (part === '..') {
      baseParts.pop()
      continue
    }
    baseParts.push(part)
  }

  return baseParts.join('/')
}

function getOpfPath(files: ZipEntry[]) {
  const opfEntry = files.find((entry) => !entry.directory && /\.opf$/i.test(entry.name))
  return opfEntry?.name ?? ''
}

function getZipObject(zip: JSZip, path: string): ZipObjectLike | null {
  return zip.file(path) as ZipObjectLike | null
}

function getBookTitle(fileName: string, opfPath: string, opfXml: string) {
  const $ = cheerio.load(opfXml, { xmlMode: true })
  const title = normalizeWhitespace(
    $('metadata > title').first().text() || $('dc\\:title').first().text() || ''
  )

  if (title) {
    return title
  }

  return opfPath
    ? opfPath.split('/').pop()?.replace(/\.opf$/i, '') ?? fileName.replace(/\.epub$/i, '')
    : fileName.replace(/\.epub$/i, '')
}

async function getSpineItems(zip: JSZip, opfPath: string) {
  const opfFile = getZipObject(zip, opfPath)
  if (!opfFile) {
    return { opfXml: '', paths: [] as string[] }
  }

  const opfXml = await opfFile.async('string')
  const $ = cheerio.load(opfXml, { xmlMode: true })
  const manifest = new Map<string, string>()

  $('manifest > item').each((_, item) => {
    const id = $(item).attr('id')
    const href = $(item).attr('href')
    const mediaType = ($(item).attr('media-type') || '').toLowerCase()

    if (!id || !href) {
      return
    }

    if (mediaType.includes('html') || mediaType.includes('xhtml') || mediaType.includes('xml')) {
      manifest.set(id, resolveRelativePath(opfPath, href))
    }
  })

  const paths = $('spine > itemref')
    .map((_, itemref) => {
      const idref = $(itemref).attr('idref')
      return idref ? manifest.get(idref) : undefined
    })
    .get()
    .filter((path): path is string => Boolean(path))

  return { opfXml, paths }
}

async function getReadableChapterPaths(zip: JSZip, files: ZipEntry[], opfPath: string) {
  const { paths: spinePaths } = opfPath ? await getSpineItems(zip, opfPath) : { opfXml: '', paths: [] }
  const fallbackPaths = files
    .filter((entry) => !entry.directory && /\.(x?html?|xml)$/i.test(entry.name))
    .map((entry) => entry.name)

  return [...spinePaths, ...fallbackPaths.filter((path) => !spinePaths.includes(path))]
}

export async function convertEpubToText(
  file: File,
  options: EpubTextOptions = {}
): Promise<EpubTextResult> {
  const buffer = await file.arrayBuffer()
  const zip = await JSZip.loadAsync(buffer)
  const files = Object.keys(zip.files).map((name) => ({
    name,
    directory: zip.files[name].dir
  }))

  const opfPath = getOpfPath(files)
  const chapterPaths = await getReadableChapterPaths(zip, files, opfPath)
  const totalCandidates = chapterPaths.length

  if (totalCandidates === 0) {
    throw new Error('No readable chapter files were found in this EPUB.')
  }

  let chapterCount = 0
  let opfXml = ''

  if (opfPath) {
    const opfFile = getZipObject(zip, opfPath)
    opfXml = opfFile ? await opfFile.async('string') : ''
  }

  const bookTitle = getBookTitle(file.name, opfPath, opfXml)

  for (const path of chapterPaths) {
    const zipFile = getZipObject(zip, path)
    if (!zipFile) {
      continue
    }

    const html = await zipFile.async('string')
    const chapter = stripHtmlToText(html)
    if (!chapter.bodyText) {
      continue
    }

    chapterCount += 1
    const chapterTitle = chapter.title || `Chapter ${chapterCount}`
    const chunkText = `${chapterTitle}. ${chapter.bodyText} `

    await options.onProgress?.({
      chapterIndex: chapterCount,
      totalCandidates,
      chapterTitle,
      fileName: file.name
    })

    await options.onChunk?.({
      chapterIndex: chapterCount,
      chapterTitle,
      text: chunkText
    })
  }

  if (chapterCount === 0) {
    throw new Error('No readable chapter text was extracted from this EPUB.')
  }

  return {
    chapterCount,
    fileCount: files.length,
    bookTitle
  }
}
