# Python Client for Realtime Arduino

This project contains Python scripts for visualizing and interacting with the Arduino realtime system.

## Prerequisites

*   Python 3.x installed.

## Standard Setup (Recommended)

If you have a standard Python installation (installed via installer from python.org):

1.  **Create Environment & Install:**
    ```bash
    python setup_env.py
    ```
2.  **Activate:**
    *   Windows: `venv\Scripts\activate`
    *   Linux/Mac: `source venv/bin/activate`
3.  **Run:**
    ```bash
    python uwb_dashboard.py
    ```

---

## Setup with Python Embedded

If you are using the **Embedded Zip** version (e.g., `python-3.14.2-embed-amd64`), standard tools like `venv` are missing. Follow these steps to make it work:

### 1. Enable `site-packages`
1.  Go to your Python Embedded folder (e.g., `E:\programs\python-3.14.2-embed-amd64\`).
2.  Open the file named `python3xx._pth` (e.g., `python314._pth`) with a text editor.
3.  Find the line `#import site` and **remove the `#`** to uncomment it.
4.  Save the file.

### 2. Install `pip`
The embedded version does not come with pip.
1.  Download [get-pip.py](https://bootstrap.pypa.io/get-pip.py) and save it to your Python folder.
2.  Open a terminal in that folder and run:
    ```cmd
    .\python.exe get-pip.py
    ```

### 3. Install Dependencies
Since `venv` is not available, install the libraries directly into the embedded Python folder:

1.  Navigate to this project folder (`pythonClient`).
2.  Run the following command using your embedded python executable:
    ```cmd
    E:\programs\python-3.14.2-embed-amd64\python.exe -m pip install -r requirements.txt
    ```

### 4. Run the Scripts
You can now run the scripts directly without activating a virtual environment:

```cmd
E:\programs\python-3.14.2-embed-amd64\python.exe uwb_dashboard.py
```

## Usage

*   **UWB Dashboard:** `python uwb_dashboard.py`
*   **UWB Map GUI:** `python uwb_map_gui.py`
