import os
import subprocess
import sys

def run_command(command, cwd=None):
    """Выполняет команду в терминале и выводит результат."""
    try:
        result = subprocess.run(command, cwd=cwd, check=True, text=True, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print(result.stdout)
        if result.stderr:
            print(result.stderr, file=sys.stderr)
    except subprocess.CalledProcessError as e:
        print(f"Ошибка выполнения команды: {e}", file=sys.stderr)
        sys.exit(1)

def main():
    # Путь к проекту
    project_dir = os.path.abspath(os.path.dirname(__file__))
    build_dir = os.path.join(project_dir, "build")

    # Убедимся, что папка для сборки существует
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    # Команда для генерации файлов сборки CMake
    cmake_command = (
        f'cmake -S "{project_dir}" -B "{build_dir}" '
        f'-G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.9.1/mingw_64" '
        f'-DCMAKE_CXX_STANDARD=20'
    )
    print("Генерация файлов сборки...")
    run_command(cmake_command)

    # Команда для сборки проекта
    build_command = f'cmake --build "{build_dir}"'
    print("Сборка проекта...")
    run_command(build_command)

    print("Сборка завершена успешно!")

if __name__ == "__main__":
    main()