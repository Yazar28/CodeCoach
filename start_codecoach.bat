@echo off
setlocal ENABLEDELAYEDEXPANSION
REM =========================================
REM  Script para arrancar TODOS los servicios
REM  Proyecto: CodeCoach (Windows)
REM =========================================

REM 1) RUTA RAÍZ DEL PROYECTO  🔽 AJÚSTALA SOLO SI MUEVES LA CARPETA
set ROOT=C:\Users\Usuario\OneDrive\Desktop\TRABAJOS\Proyectos Datos II\codecoach

REM Carpetas derivadas
set SERVICES=%ROOT%\services
set UI_DIR=%ROOT%\ui

echo ========================================
echo  Iniciando servicios de CodeCoach...
echo  ROOT = %ROOT%
echo ========================================
echo.

REM 2) MongoDB standalone con mongod.exe (NO servicio de Windows)
echo [MongoDB] Iniciando mongod.exe...
IF NOT EXIST "C:\data\db" (
    echo  - Creando carpeta de datos en C:\data\db
    mkdir "C:\data\db"
)
start "mongod" /D "C:\Program Files\MongoDB\Server\8.2\bin" cmd /k mongod.exe --dbpath C:\data\db
echo.

REM 3) Microservicio Python: mongo_manager.py  (Mongo + Problems)
echo [Python] Iniciando mongo_manager.py...
start "mongo_manager" /D "%SERVICES%\microservices" cmd /k py mongo_manager.py
echo.

REM 4) Microservicio Python: llm_proxy.py  (LLM / Analyzer)
echo [Python] Iniciando llm_proxy.py...
start "llm_proxy" /D "%SERVICES%\microservices" cmd /k py llm_proxy.py
echo.

REM 5) Evaluator (C++) en 8082
REM    Ejecutable en: services\evaluator\out\build\x64-Debug\evaluator.exe
echo [C++] Iniciando Evaluator...
start "evaluator" /D "%SERVICES%\evaluator\out\build\x64-Debug" cmd /k evaluator.exe
echo.

REM 6) Analyzer (C++) en 8083
REM    Ejecutable en: services\analyzer\out\build\x64-Debug\analyzer.exe
echo [C++] Iniciando Analyzer...
start "analyzer" /D "%SERVICES%\analyzer\out\build\x64-Debug" cmd /k analyzer.exe
echo.

REM 7) Problem Manager C++ (REST API) en 8084
REM    Ejecutable en: services\problem-manager\out\build\x64-Debug\problem_manager.exe
echo [C++] Iniciando Problem Manager C++...
start "problem-manager" /D "%SERVICES%\problem-manager\out\build\x64-Debug" cmd /k problem_manager.exe
echo.

REM 8) UI (React / Vite) - npm run dev
echo [UI] Iniciando interfaz (npm run dev)...
start "ui" /D "%UI_DIR%" cmd /k npm run dev
echo.

echo ==========================================
echo  Todos los comandos fueron lanzados.
echo  Revisa las ventanas para posibles errores.
echo ==========================================
pause
endlocal

