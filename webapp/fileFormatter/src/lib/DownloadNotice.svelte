<script lang="ts">
  export let visible = false
  export let message = ''
  export let downloadUrl = ''
  export let downloadName = 'processed.txt'
</script>

{#if visible && downloadUrl}
  <section class="notice" aria-live="polite">
    <div>
      <h2>Ready</h2>
      <p>{message}</p>
    </div>

    <a class="download" href={downloadUrl} download={downloadName}>Download .txt</a>
  </section>
{/if}
