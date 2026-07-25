<script lang="ts">
  import { onDestroy } from 'svelte'
  import { convertEpubToText } from './lib/epub'
  import DownloadNotice from './lib/DownloadNotice.svelte'

  let fileName = ''
  let status = 'Drop an EPUB file to begin.'
  let downloadUrl = ''
  let downloadName = ''

  function toSafeFileName(value: string) {
    return value.replace(/[\\/:*?"<>|]+/g, '_').trim()
  }

  function resetDownloadUrl() {
    if (downloadUrl) {
      URL.revokeObjectURL(downloadUrl)
      downloadUrl = ''
    }
  }

  onDestroy(() => {
    resetDownloadUrl()
  })

  async function processFile(file: File | undefined) {
    if (!file) return

    resetDownloadUrl()
    fileName = file.name
    status = 'Processing EPUB...'
    downloadName = ''

    const blobParts: BlobPart[] = []

    try {
      const result = await convertEpubToText(file, {
        onProgress: ({ chapterIndex, totalCandidates, chapterTitle }) => {
          status = `Processing chapter ${chapterIndex} of ${totalCandidates}: ${chapterTitle}`
        },
        onChunk: ({ text }) => {
          blobParts.push(text)
        }
      })

      const blob = new Blob(blobParts, { type: 'text/plain;charset=utf-8' })

      downloadUrl = URL.createObjectURL(blob)
      downloadName = `${toSafeFileName(result.bookTitle || file.name.replace(/\.epub$/i, '') || 'processed-book')}.txt`
      status = `Processed ${result.chapterCount} chapters from ${file.name}.`
    } catch (error) {
      console.error(error)
      status = error instanceof Error ? error.message : 'Failed to process EPUB.'
      downloadName = ''
    }
  }

  function handleDrop(event: DragEvent) {
    event.preventDefault()
    const dt = event.dataTransfer
    processFile(dt?.files?.[0])
  }

  function handleInput(event: Event) {
    const input = event.currentTarget as HTMLInputElement
    processFile(input.files?.[0])
  }
</script>

<svelte:head>
  <title>EPUB Processor</title>
</svelte:head>

<main class="app">
  <section class="panel">
    <h1>EPUB Processor</h1>
    <p class="status">{status}</p>

    <label class="dropzone" on:drop={handleDrop} on:dragover|preventDefault>
      <input type="file" accept=".epub" on:change={handleInput} />
      <strong>Drop EPUB here</strong>
      <span>or click to choose a file</span>
    </label>

    {#if fileName}
      <p class="file">Selected: {fileName}</p>
    {/if}

    <DownloadNotice
      visible={Boolean(downloadUrl)}
      message="EPUB processed successfully. Your cleaned text file is ready."
      downloadUrl={downloadUrl}
      downloadName={downloadName}
    />
  </section>
</main>
