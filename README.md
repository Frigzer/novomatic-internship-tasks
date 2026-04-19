# novomatic-internship-tasks

## Wymagania wstępne

* CMake ≥ 3.23
* Conan 2.x

## Build

```bash
conan profile detect --force
```

### Release

```bash
conan install . -pr:h=profiles/release.conan -pr:b=profiles/release.conan --build=missing
cmake --preset conan-release
cmake --build --preset conan-release
ctest --preset conan-release
```

---

### Debug

```bash
conan install . -pr:h=profiles/debug.conan -pr:b=profiles/debug.conan --build=missing
cmake --preset conan-debug
cmake --build --preset conan-debug
ctest --preset conan-debug
```

## Uwagi

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
