SIMPLE CHEATS: мод для Geometry Dash (Geode 5.11.2, GD 2.2081)
==============================================================

В архиве исходники мода. Готового файла .geode здесь нет: его надо собрать.
Проще всего собрать бесплатно на GitHub, ставить ничего не нужно.

Что внутри
- src/main.cpp ............. код (noclip, speedhack, replay)
- mod.json ................. описание и настройки мода
- CMakeLists.txt ........... сборка
- logo.png, about.md ....... иконка и описание в списке модов
- changelog.md ............. список изменений
- .github/workflows/build.yml ... автосборка под Android (arm64)

Как собрать
1. Зарегистрируйтесь на github.com (бесплатно) и создайте новый репозиторий:
   «+» -> New repository -> любое имя -> Create repository.
2. Загрузите в него файлы проекта.
   - С компьютера: Add file -> Upload files, перетащите всё содержимое папки
     simple-cheats вместе с папками src и .github.
   - С телефона: включите в браузере «Версия для ПК». Файлы из корня (mod.json,
     CMakeLists.txt, about.md, changelog.md, logo.png, .gitignore) загрузите через
     Add file -> Upload files. Файлы в папках создайте через Add file -> Create new file:
     в имени напишите src/main.cpp и вставьте содержимое, затем так же
     .github/workflows/build.yml. Слеш в имени сам создаёт папку.
3. Откройте вкладку Actions. Если GitHub просит, нажмите кнопку включения.
   Сборка запустится сама после загрузки файлов (или запустите вручную:
   Build Android -> Run workflow).
4. Через несколько минут откройте завершённый запуск и внизу скачайте
   «Build Output». Внутри лежит файл max.simple-cheats.geode.
5. Положите .geode в папку модов Geode. Обычно это
   Android/media/com.geode.launcher/game/geode/mods/
   Перезапустите игру.
6. В Geode откройте Simple Cheats -> настройки и включите нужные читы.

Если сборка упала с ошибкой, скопируйте текст ошибки из Actions и пришлите.
