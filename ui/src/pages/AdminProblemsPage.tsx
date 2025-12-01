import { useState } from 'react'
import { useQuery, useMutation, useQueryClient } from '@tanstack/react-query'
import {
  createProblem,
  deleteProblem,
  listProblems,
  type ProblemSummary,
} from '../api/clients'
import type { CreateProblemReq } from '../types'

type Tab = 'create' | 'manage'

export default function AdminProblemsPage() {
  const [activeTab, setActiveTab] = useState<Tab>('create')

  // --- Estados para crear problema ---
  const [id, setId] = useState('')
  const [title, setTitle] = useState('')
  const [difficulty, setDifficulty] = useState<'easy' | 'medium' | 'hard'>('easy')
  const [tags, setTags] = useState('array, hash-table')
  const [statement, setStatement] = useState('')
  const [examplesRaw, setExamplesRaw] = useState(
    `[
  { "in": "nums = [2,7,11,15], target = 9", "out": "[0,1]" }
]`,
  )
  const [starterCode, setStarterCode] = useState(
    `class Solution {
public:
    // tu solución aquí
};`,
  )
  const [testsRaw, setTestsRaw] = useState(
    `[
  { "in": { "nums": [2,7,11,15], "target": 9 }, "out": "[0,1]" }
]`,
  )
  const [status, setStatus] = useState<string | null>(null)
  const [error, setError] = useState<string | null>(null)

  const qc = useQueryClient()

  // --- Query para la pestaña de gestión / eliminación ---
  const problemsQuery = useQuery<ProblemSummary[]>({
    queryKey: ['admin-problems'],
    queryFn: listProblems,
    enabled: activeTab === 'manage',
  })

  const deleteMut = useMutation({
    mutationFn: (pid: string) => deleteProblem(pid),
    onSuccess: () => {
      qc.invalidateQueries({ queryKey: ['admin-problems'] })
      qc.invalidateQueries({ queryKey: ['problems'] })
    },
  })

  // --- Handler de creación ---
  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setStatus(null)
    setError(null)

    try {
      const examples = JSON.parse(examplesRaw)
      const tests = JSON.parse(testsRaw)

      if (!Array.isArray(examples) || !Array.isArray(tests)) {
        throw new Error('Ejemplos y tests deben ser arreglos JSON')
      }

      const payload: CreateProblemReq = {
        id: id.trim(),
        title: title.trim(),
        difficulty,
        tags: tags
          .split(',')
          .map(t => t.trim())
          .filter(Boolean),
        statement,
        examples,
        starterCode,
        tests,
      }

      if (!payload.id) {
        throw new Error('El campo ID es obligatorio')
      }

      await createProblem(payload)
      setStatus(`Problema "${payload.id}" creado correctamente.`)

      // Si quieres limpiar campos al crear, puedes descomentar:
      // setId('')
      // setTitle('')
      // setStatement('')

      qc.invalidateQueries({ queryKey: ['problems'] })
      qc.invalidateQueries({ queryKey: ['admin-problems'] })
    } catch (err: any) {
      console.error(err)
      setError(err?.message ?? 'Error al crear el problema')
    }
  }

  // --- UI ---
  return (
    <div style={{ padding: 16 }}>
      <h1 style={{ marginBottom: 8 }}>Admin de problemas</h1>
      <p style={{ fontSize: 13, color: '#555', marginBottom: 16 }}>
        Desde aquí puedes crear nuevos problemas o gestionar / eliminar existentes.
      </p>

      {/* Tabs */}
      <div style={{ display: 'flex', gap: 8, marginBottom: 16, borderBottom: '1px solid #eee' }}>
        <button
          type="button"
          onClick={() => setActiveTab('create')}
          style={{
            padding: '6px 12px',
            border: 'none',
            borderBottom: activeTab === 'create' ? '2px solid #1976d2' : '2px solid transparent',
            background: 'transparent',
            cursor: 'pointer',
            fontWeight: activeTab === 'create' ? 600 : 400,
          }}
        >
          Crear problema
        </button>
        <button
          type="button"
          onClick={() => setActiveTab('manage')}
          style={{
            padding: '6px 12px',
            border: 'none',
            borderBottom: activeTab === 'manage' ? '2px solid #1976d2' : '2px solid transparent',
            background: 'transparent',
            cursor: 'pointer',
            fontWeight: activeTab === 'manage' ? 600 : 400,
          }}
        >
          Gestionar / eliminar problemas
        </button>
      </div>

      {activeTab === 'create' ? (
        <form onSubmit={handleSubmit} style={{ display: 'grid', gap: 12, maxWidth: 800 }}>
          <div style={{ display: 'grid', gap: 4 }}>
            <label>ID (único)</label>
            <input
              value={id}
              onChange={e => setId(e.target.value)}
              placeholder="two-sum, reverse-string, count-negatives, etc."
              style={{ padding: 6 }}
            />
          </div>

          <div style={{ display: 'grid', gap: 4 }}>
            <label>Título</label>
            <input
              value={title}
              onChange={e => setTitle(e.target.value)}
              placeholder="Two Sum"
              style={{ padding: 6 }}
            />
          </div>

          <div style={{ display: 'grid', gap: 4 }}>
            <label>Dificultad</label>
            <select
              value={difficulty}
              onChange={e => setDifficulty(e.target.value as typeof difficulty)}
              style={{ padding: 6, maxWidth: 200 }}
            >
              <option value="easy">easy</option>
              <option value="medium">medium</option>
              <option value="hard">hard</option>
            </select>
          </div>

          <div style={{ display: 'grid', gap: 4 }}>
            <label>Tags (separados por coma)</label>
            <input
              value={tags}
              onChange={e => setTags(e.target.value)}
              placeholder="array, hash-table"
              style={{ padding: 6 }}
            />
          </div>

          <div style={{ display: 'grid', gap: 4 }}>
            <label>Enunciado</label>
            <textarea
              value={statement}
              onChange={e => setStatement(e.target.value)}
              rows={4}
              style={{ padding: 6 }}
            />
          </div>

          <div style={{ display: 'grid', gap: 4 }}>
            <label>Ejemplos (JSON)</label>
            <textarea
              value={examplesRaw}
              onChange={e => setExamplesRaw(e.target.value)}
              rows={6}
              style={{ fontFamily: 'monospace', padding: 6 }}
            />
          </div>

          <div style={{ display: 'grid', gap: 4 }}>
            <label>Starter code (C++)</label>
            <textarea
              value={starterCode}
              onChange={e => setStarterCode(e.target.value)}
              rows={8}
              style={{ fontFamily: 'monospace', padding: 6 }}
            />
          </div>

          <div style={{ display: 'grid', gap: 4 }}>
            <label>Tests (JSON)</label>
            <textarea
              value={testsRaw}
              onChange={e => setTestsRaw(e.target.value)}
              rows={6}
              style={{ fontFamily: 'monospace', padding: 6 }}
            />
          </div>

          <button type="submit" style={{ padding: '8px 16px' }}>
            Crear problema
          </button>

          {status && <div style={{ color: 'green' }}>{status}</div>}
          {error && <div style={{ color: 'crimson', whiteSpace: 'pre-wrap' }}>{error}</div>}

          <p style={{ fontSize: 12, color: '#666' }}>
            Tip: después de crear un problema nuevo, puedes ir a la página principal, recargar y
            debería aparecer en la lista.
          </p>
        </form>
      ) : (
        <div>
          <h2 style={{ marginBottom: 8 }}>Problemas existentes</h2>

          {problemsQuery.isLoading && <div>Cargando problemas…</div>}
          {problemsQuery.isError && (
            <div style={{ color: 'crimson' }}>
              Error al cargar problemas: {(problemsQuery.error as Error)?.message ?? 'desconocido'}
            </div>
          )}

          {problemsQuery.data && problemsQuery.data.length === 0 && (
            <div style={{ fontSize: 14, color: '#555' }}>No hay problemas registrados.</div>
          )}

          {problemsQuery.data && problemsQuery.data.length > 0 && (
            <table
              style={{
                borderCollapse: 'collapse',
                fontSize: 14,
                minWidth: 600,
              }}
            >
              <thead>
                <tr>
                  <th style={{ textAlign: 'left', borderBottom: '1px solid #ddd', padding: 6 }}>ID</th>
                  <th style={{ textAlign: 'left', borderBottom: '1px solid #ddd', padding: 6 }}>Título</th>
                  <th style={{ textAlign: 'left', borderBottom: '1px solid #ddd', padding: 6 }}>Dificultad</th>
                  <th style={{ textAlign: 'left', borderBottom: '1px solid #ddd', padding: 6 }}>Tags</th>
                  <th style={{ textAlign: 'left', borderBottom: '1px solid #ddd', padding: 6 }}>Acciones</th>
                </tr>
              </thead>
              <tbody>
                {problemsQuery.data.map(p => (
                  <tr key={p.id}>
                    <td style={{ borderBottom: '1px solid #f0f0f0', padding: 6 }}>{p.id}</td>
                    <td style={{ borderBottom: '1px solid #f0f0f0', padding: 6 }}>{p.title}</td>
                    <td style={{ borderBottom: '1px solid #f0f0f0', padding: 6 }}>{p.difficulty}</td>
                    <td style={{ borderBottom: '1px solid #f0f0f0', padding: 6 }}>
                      {p.tags && p.tags.length > 0 ? p.tags.join(', ') : '—'}
                    </td>
                    <td style={{ borderBottom: '1px solid #f0f0f0', padding: 6 }}>
                      <button
                        type="button"
                        onClick={() => {
                          if (!window.confirm(`¿Seguro que quieres eliminar el problema "${p.id}"?`)) {
                            return
                          }
                          deleteMut.mutate(p.id)
                        }}
                        disabled={deleteMut.isPending}
                        style={{
                          padding: '4px 10px',
                          fontSize: 12,
                          background: '#e53935',
                          color: 'white',
                          border: 'none',
                          borderRadius: 3,
                          cursor: 'pointer',
                        }}
                      >
                        {deleteMut.isPending ? 'Eliminando…' : 'Eliminar'}
                      </button>
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          )}

          <p style={{ fontSize: 12, color: '#666', marginTop: 8 }}>
            Nota: esta eliminación es permanente en la base de datos MongoDB.
          </p>
        </div>
      )}
    </div>
  )
}
