import Editor from '@monaco-editor/react'

export default function CodeEditor({
  language, value, onChange
}: { language:'cpp'|'python'|'java'; value:string; onChange:(v:string|undefined)=>void }) {
  const monacoLang = language === 'cpp' ? 'cpp' : language === 'python' ? 'python' : 'java'
  return (
    <div style={{ border:'1px solid #ddd', borderRadius:8, overflow:'hidden' }}>
      <Editor height="360px" language={monacoLang} value={value} onChange={onChange}
        options={{ fontSize:14, minimap:{ enabled:false } }} />
    </div>
  )
}
