@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   Aetheris Platform - Windows Build ^& Run
echo ===================================================

:: Check for Java
java -version >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Java not found. Please install Java 21 or later.
    exit /b 1
)

:: Check for Node.js
node -v >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] Node.js not found. Please install Node.js 18 or later.
    exit /b 1
)

echo.
echo [1/3] Building Backend Services...
call gradlew.bat build -x test
if %errorlevel% neq 0 (
    echo [ERROR] Backend build failed.
    exit /b 1
)

echo.
echo [2/3] Building Frontend Application...
cd frontend
call npm install --silent
call npm run build
cd ..

echo.
echo [3/3] Launching Aetheris Services...
echo.
echo Launching Streaming Core (Port 8081)...
start "Aetheris - Streaming Core" java -jar backend/streaming-core/build/libs/streaming-core-0.0.1-SNAPSHOT.jar

echo Launching GraphQL API (Port 8080)...
start "Aetheris - GraphQL API" java -jar backend/graphql-api/build/libs/graphql-api-0.0.1-SNAPSHOT.jar

echo Launching Ingestion Gateway (Port 8082)...
start "Aetheris - Ingestion Gateway" java -jar backend/ingestion/build/libs/ingestion-0.0.1-SNAPSHOT.jar

echo Launching Frontend Development Server...
cd frontend
start "Aetheris - Frontend" npm run dev
cd ..

echo.
echo ===================================================
echo   Services are starting in separate windows.
echo   - Frontend: http://localhost:5173
echo   - GraphQL: http://localhost:8080/graphiql
echo   - Streaming: ws://localhost:8081/ws/telemetry
echo ===================================================
pause
