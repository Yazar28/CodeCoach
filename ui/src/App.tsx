import Editor from '@monaco-editor/react'

export default function App() {
  return (
    <div style={{ padding: 16, fontFamily: 'system-ui, Arial' }}>
      <h1>Smoke test Monaco ✅</h1>
      <p>Si ves el editor abajo, Monaco está bien instalado.</p>
      <Editor
        height="260px"
        defaultLanguage="cpp"
        defaultValue={`#include <bits/stdc++.h>\nusing namespace std;\nint main(){return 0;}`}
        options={{ minimap: { enabled: false }, fontSize: 14 }}
      />
    </div>
  )
}
