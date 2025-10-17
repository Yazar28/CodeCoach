// src/App.tsx
import Editor from '@monaco-editor/react'
import { Link } from 'react-router-dom'

export default function App() {
  return (
    <div style={{ minHeight: '100vh', display: 'flex', flexDirection: 'column', fontFamily: 'system-ui, Arial' }}>
      <header style={{ padding: '12px 16px', borderBottom: '1px solid #eee', display: 'flex', gap: 16, alignItems: 'center' }}>
        <Link to="/" style={{ fontWeight: 700, textDecoration: 'none', color: 'inherit' }}>CodeCoach</Link>
        <nav style={{ display: 'flex', gap: 12, fontSize: 14 }}>
          {/* Cuando creemos las páginas, aquí irán los links reales */}
          <span style={{ color: '#666' }}>Problemas (pronto)</span>
          <span style={{ color: '#666' }}>Envíos (pronto)</span>
        </nav>
      </header>

      <main style={{ padding: 16, maxWidth: 1000, margin: '0 auto', width: '100%', flex: 1 }}>
        <h1 style={{ fontSize: 22, marginBottom: 8 }}>Base de la UI lista ✅</h1>
        <p style={{ marginBottom: 16 }}>
          Tenemos Router + React Query configurados. A continuación añadiremos las páginas.
          Por ahora, este editor confirma que <b>Monaco</b> sigue funcionando.
        </p>

        <div style={{ border: '1px solid #ddd', borderRadius: 8, overflow: 'hidden' }}>
          <Editor
            height="260px"
            defaultLanguage="cpp"
            defaultValue={`#include <bits/stdc++.h>\\nusing namespace std;\\nint main(){return 0;}`}
            options={{ minimap: { enabled: false }, fontSize: 14 }}
          />
        </div>
      </main>

      <footer style={{ padding: '12px 16px', borderTop: '1px solid #eee', fontSize: 12, color: '#666' }}>
        MVP intermedio — listo para agregar páginas (Problems, ProblemDetail, Submission).
      </footer>
    </div>
  )
}
