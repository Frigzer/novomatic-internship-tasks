# novomatic-internship-tasks

## Instalacja Conana

### Windows

Opcja 1 (polecana):

```bash
winget install JFrog.Conan
```

Opcja 2:

```bash
pip install conan
```

### Linux 

Najprościej przez pip:

```bash
pip install --user conan
```

Jeśli `pip` nie jest dostępny:

```bash
sudo apt install python3-pip
pip3 install --user conan
```

Dodaj do PATH jeśli trzeba:

```bash
export PATH="$HOME/.local/bin:$PATH"
```

---

## Build

### Windows

```bash
conan install . -pr:h=profiles/windows-clang-release -pr:b=profiles/windows-clang-release --build=missing
cmake --preset conan-default
cmake --build --preset conan-release
ctest --preset conan-release
```

---

### Linux

```bash
conan install . -pr:h=profiles/linux-clang-release -pr:b=profiles/linux-clang-release --build=missing
cmake --preset conan-default
cmake --build --preset conan-release
ctest --preset conan-release
```

### VS Code

Projekt korzysta z **CMake presets generowanych przez Conana**.

Ustaw w `.vscode/settings.json`:

```json
{
  "cmake.useCMakePresets": "always"
}
```

Potem:

1. `conan install ...`
2. `CMake: Select Configure Preset`
3. `CMake: Select Build Preset`
4. build

---

## Uwagi

* Nie używaj `conan profile detect` jako głównego profilu — profile są w `profiles/`
* Zawsze używaj:

  ```bash
  -pr:h=<profil> -pr:b=<profil>
  ```
---

## Struktura

```text
profiles/        # profile Conan (Windows/Linux)
task1/           # implementacja zadania 1
task2/           # implementacja zadania 2
task3/           # implementacja zadania 3
conanfile.txt    # zależności
CMakeLists.txt   # root
```
