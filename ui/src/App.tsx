import { Link, NavLink, Route, Routes } from 'react-router-dom'
import ProblemsPage from './pages/ProblemsPage'
import ProblemDetailPage from './pages/ProblemDetailPage'
import SubmissionPage from './pages/SubmissionPage'

export default function App() {
  return (
    <div style={{ minHeight:'100vh', display:'flex', flexDirection:'column', fontFamily:'system-ui, Arial' }}>
      <header style={{ padding:'12px 16px', borderBottom:'1px solid #eee', display:'flex', gap:16, alignItems:'center' }}>
        <Link to="/" style={{ fontWeight:700, textDecoration:'none', color:'inherit' }}>CodeCoach</Link>
        <nav style={{ display:'flex', gap:12, fontSize:14 }}>
          <NavLink to="/" style={({isActive})=>({ textDecoration: isActive ? 'underline' : 'none' })}>Problemas</NavLink>
        </nav>
      </header>

      <main style={{ padding:16, maxWidth:1000, margin:'0 auto', width:'100%', flex:1 }}>
        <Routes>
          <Route path="/" element={<ProblemsPage />} />
          <Route path="/problems/:id" element={<ProblemDetailPage />} />
          <Route path="/submissions/:id" element={<SubmissionPage />} />
        </Routes>
      </main>

      <footer style={{ padding:'12px 16px', borderTop:'1px solid #eee', fontSize:12, color:'#666' }}>
        MVP de práctica — Monaco + React Query + Routing (mocks).
      </footer>
    </div>
  )
}
