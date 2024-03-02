
msbuild.exe engine.sln /p:Configuration=Release -t:Clean
if errorlevel not 0 echo error: %errorlevel% && exit /b errorlevel

msbuild.exe engine.sln /p:Configuration=Release -target:utils\xrai;utils\xrlc;utils\xrdo_light -m
if %errorlevel% neq 0 echo error: %errorlevel% && exit /b errorlevel

msbuild.exe engine.sln /p:Configuration=Release -target:editors\LevelEditor -m
if %errorlevel% neq 0 echo error: %errorlevel% && exit /b errorlevel

msbuild.exe engine.sln /p:Configuration=Release -target:xrgame -m
if %errorlevel% neq 0 echo error: %errorlevel% && exit /b errorlevel
