# CodeCoach

> Plataforma didáctica para practicar problemas tipo entrevista (estilo LeetCode/HackerRank) con UI web y servicios C++.

<div align="center">

**Estado del MVP**

| Módulo                               | Estado                                                     |
| ------------------------------------ | ---------------------------------------------------------- |
| UI (React + TS + Monaco)             | ✅ Base + flujo mock (lista → detalle → envío → resultados) |
| Problem Manager (C++ REST :8081)     | 🟨 Planeado (siguiente paso)                               |
| Evaluator (C++ REST :8082 + sandbox) | ⬜ Pendiente                                                |
| Analyzer (REST :8083)                | ⬜ Pendiente                                                |

</div>

---

## ✨ Funcionalidades (MVP actual)

* Lista de problemas y navegación al detalle.
* Editor de código integrado (**Monaco**).
* Envío de solución y vista de resultados con *polling*.
* Panel de "feedback del coach" (mock).

> En esta etapa, la UI usa **mocks**. Cuando el backend esté listo, se desactiva el modo mock y se conectan las APIs reales.

---

## 🧱 Arquitectura (visión)

```
[UI React] --HTTP--> [Problem Manager :8081] --DB--> [Mongo]
      \--HTTP--> [Evaluator :8082] --(sandbox)--> [Runner]
       \-HTTP-> [Analyzer :8083] --(LLM)--> [Proveedor IA]
```

---

## 🛠️ Tecnologías

* **UI:** Vite, React, TypeScript, React Router, React Query, Monaco Editor, Axios
* **Servicios (planeados):** C++17/20, cpp-httplib o Pistache, nlohmann-json, MongoDB C++ (mongocxx), fmt
* **Herramientas:** GitHub Desktop, Git Bash/PowerShell, VS Code, Visual Studio + CMake, vcpkg

---

## 📁 Estructura del repositorio

```
codecoach/
  ui/
  services/
    problem-manager/
    evaluator/
    analyzer/
  shared/
  docs/
  scripts/
  .editorconfig
  .gitattributes
  .gitignore
  README.md
```

---

## ⚙️ Requisitos previos

* **Node.js 20+** (ej.: v22.x) y **npm 10+**
* **Git** y **GitHub Desktop**
* (Opcional para servicios) **Visual Studio** con toolset C++ y **CMake** / **vcpkg**

---

## 🚀 Puesta en marcha (UI)

```bash
# 1) Entrar en la carpeta de UI
cd ui

# 2) Instalar dependencias (si no se hizo ya)
npm install

# 3) Ejecutar en desarrollo
npm run dev
# → abre http://localhost:5173
```

### Librerías UI que ya usamos

```bash
npm i @tanstack/react-query axios @monaco-editor/react react-router-dom
```

---

## 🔧 Variables de entorno (UI)

Crear `ui/.env.local` cuando conectemos backend real:

```ini
VITE_API_PM=http://localhost:8081
VITE_API_EVAL=http://localhost:8082
VITE_API_ANALYZER=http://localhost:8083
```

### Activar/desactivar mocks

En `src/api/clients.ts`:

```ts
const USE_MOCK = true; // ← UI con datos simulados (actual)
// Cambiar a false para usar APIs reales
```

---

## 📜 Contratos REST (resumen)

**Problem Manager (PM)**

* `GET /problems?tag=&difficulty=` → lista (resumen)
* `GET /problems/{id}` → detalle del problema

**Evaluator**

* `POST /submissions` → `{ submissionId }`
* `GET /submissions/{id}` → `{ status, results[], timeMs, memoryKB, compileErrors? }`

**Analyzer**

* `POST /analysis` → `{ hints[], probablePatterns?, complexityEstimate? }`

> Nota: Estos endpoints están **planificados** para el backend; la UI ya está preparada para consumirlos.

---

## 🌿 Flujo de trabajo con GitHub Desktop

1. **Crear rama:** `Current Branch → New Branch…` (ej.: `feature/pm-problem-manager`).
2. **Commit en español:** `feat: ...`, `fix: ...`, `docs: ...`, `chore: ...`.
3. **Push** de la rama → **Create Pull Request** (modo *Draft* hasta validar).
4. **Merge** a `main` con **Squash & merge**.

> Regla recomendada: no *push* directo a `main`.

---

## 🧪 Pruebas rápidas (UI)

* Abrir `http://localhost:5173` → ver **lista de problemas** (mock).
* Entrar a **Two Sum** → aparece **editor**.
* **Enviar** → ver **Resultados** con *polling* y feedback.

---

## 🧰 Solución de problemas (Windows)

* **CRLF/LF warnings:** inofensivos. Se pueden normalizar con `.gitattributes` y `git add --renormalize .`.
* **Puerto ocupado:** cambia `5173` con `--port` o libera proceso en conflicto.
* **Monaco no carga:** asegúrate de tener `@monaco-editor/react` instalado y reinicia `npm run dev`.
* **Rutas con espacios (OneDrive):** usa comillas en la terminal `"C:/Users/.../Proyectos Datos II/..."`.

---

## 🗺️ Roadmap (corto plazo)

* [ ] **PM** (C++): `GET /problems`, `GET /problems/{id}`.
* [ ] Conectar UI a PM (desactivar mock solo para PM).
* [ ] **Evaluator** (C++ + sandbox): `POST/GET /submissions` + límites de tiempo/memoria.
* [ ] **Analyzer**: feedback no‑spoiler.
* [ ] Documentación y demo E2E.

---

## 📄 Licencia

Pendiente de definir.

## 🙌 Créditos

Equipo del curso (2024/2025).
