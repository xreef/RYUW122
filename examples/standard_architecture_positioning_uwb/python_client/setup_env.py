import os
import subprocess
import sys

def create_venv(venv_dir):
    """Creates a virtual environment."""
    print(f"Creating virtual environment in {venv_dir}...")
    try:
        import venv
        venv.create(venv_dir, with_pip=True)
    except ImportError:
        print("Warning: 'venv' module not found (common in embedded Python).")
        print("Attempting fallback to 'virtualenv'...")
        try:
            subprocess.check_call([sys.executable, "-m", "virtualenv", venv_dir])
        except subprocess.CalledProcessError:
            print("Error: Both 'venv' and 'virtualenv' are unavailable.")
            print("Please use a full Python installation or install 'virtualenv'.")
            sys.exit(1)
    except Exception as e:
        print(f"Error creating virtual environment: {e}")
        sys.exit(1)

def install_requirements(venv_dir, requirements_file):
    """Installs requirements from a file into the virtual environment."""
    if sys.platform == "win32":
        pip_executable = os.path.join(venv_dir, "Scripts", "pip")
    else:
        pip_executable = os.path.join(venv_dir, "bin", "pip")
    
    # Verify pip exists before trying to use it
    if not os.path.exists(pip_executable) and not os.path.exists(pip_executable + ".exe"):
        print(f"Error: pip executable not found at {pip_executable}")
        print("The virtual environment may have been created without pip.")
        sys.exit(1)

    print(f"Installing requirements from {requirements_file}...")
    subprocess.check_call([pip_executable, "install", "-r", requirements_file])

def main():
    venv_dir = "venv"
    requirements_file = "requirements.txt"

    if not os.path.exists(venv_dir):
        create_venv(venv_dir)
    else:
        print(f"Virtual environment '{venv_dir}' already exists.")

    if os.path.exists(requirements_file):
        install_requirements(venv_dir, requirements_file)
        print("Setup complete. To activate the virtual environment:")
        if sys.platform == "win32":
            print(f"    {venv_dir}\\Scripts\\activate")
        else:
            print(f"    source {venv_dir}/bin/activate")
    else:
        print(f"Error: {requirements_file} not found.")

if __name__ == "__main__":
    main()