// ui/src/pages/ProblemsPage.tsx
import { useQuery } from "@tanstack/react-query";
import { Link } from "react-router-dom";
import { listProblems, type ProblemSummary } from "../api/clients";

export default function ProblemsPage() {
  const { data, isLoading, isError, error } = useQuery<ProblemSummary[]>({
    queryKey: ["problems"],
    queryFn: listProblems,
  });

  if (isLoading) return <div style={{ padding: 16 }}>Cargando problemas…</div>;

  if (isError) {
    const msg = (error as Error)?.message ?? "Error desconocido";
    return <div style={{ padding: 16, color: "crimson" }}>Error: {msg}</div>;
  }

  if (!data || data.length === 0) {
    return (
      <div style={{ padding: 16 }}>
        No hay problemas (¿PM corriendo en :8081?).
      </div>
    );
  }

  return (
    <div style={{ padding: 16 }}>
      <h1 style={{ marginBottom: 12 }}>Problemas</h1>
      <ul
        style={{
          display: "grid",
          gap: 8,
          listStyle: "none",
          padding: 0,
        }}
      >
        {data.map((p) => (
          <li
            key={p.id}
            style={{
              border: "1px solid #ddd",
              borderRadius: 8,
              padding: 12,
            }}
          >
            <div style={{ fontWeight: 600, marginBottom: 4 }}>
              <Link to={`/problems/${p.id}`}>{p.title}</Link>
            </div>
            <div style={{ fontSize: 12, opacity: 0.8 }}>
              Dificultad: {p.difficulty} · Tags: {p.tags.join(", ")}
            </div>
          </li>
        ))}
      </ul>
    </div>
  );
}
