rem msbuild.exe engine.sln /p:Configuration=Release -t:Clean -target:xrgame -m
rem msbuild.exe engine.sln /p:Configuration=Release -t:Clean -target:utils\xrai;utils\xrlc;utils\xrdo_light -m
msbuild.exe engine.sln /p:Configuration=Release -t:Clean -target:editors\LevelEditor -m
