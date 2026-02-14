# ; Project Handling
# ; create_project(self, project_name: str) → Creates a new shader project (a folder).

# ; delete_project(self, project_name: str) → Deletes an existing shader project.

# ; list_projects(self) -> list → Returns a list of all available projects.

# ; open_project(self, project_name: str) → Opens an existing shader project.

# ; rename_project(self, old_name: str, new_name: str) → Renames an existing project.

# ; 2️⃣ File Management
# ; delete_shader_file(self, project_name: str, file_name: str) → Removes a shader file from a project.

# ; list_shader_files(self, project_name: str) -> list → Lists all shader files within a project.

# ; open_shader_file(self, project_name: str, file_name: str) -> str → Opens a shader file and returns its contents.

# ; rename_shader_file(self, project_name: str, old_name: str, new_name: str) → Renames an existing shader file.

# ; 3️⃣ Project Cleanup & Maintenance
# ; reset_project(self, project_name: str) → Deletes all shader files in a project but keeps the project itself.

import os
import shutil
import json

class ProjectManager:
    CONFIG_FILE = "config.json"

    def __init__(self):
        self.base_directory = self.get_base_directory()
        os.makedirs(self.base_directory, exist_ok=True)

    def get_base_directory(self):
        if os.path.exists(self.CONFIG_FILE):
            with open(self.CONFIG_FILE, "r") as file:
                config = json.load(file)
                if "base_directory" in config and os.path.exists(config["base_directory"]):
                    return config["base_directory"]

        # Ask for input if config.json is missing or invalid
        return self.ask_for_base_directory()


    def ask_for_base_directory(self):
        base_directory = input("Enter the base directory for projects: ").strip()
        if not os.path.exists(base_directory):
            os.makedirs(base_directory)
        with open(self.CONFIG_FILE, "w") as file:
            json.dump({"base_directory": base_directory}, file)
        return base_directory

    def create_project(self, project_name: str):
        project_path = os.path.join(self.base_directory, project_name)
        if not os.path.exists(project_path):
            os.makedirs(project_path)
            print(f"✅ Project '{project_name}' created at {project_path}.")
        else:
            print(f"⚠️ Project '{project_name}' already exists at {project_path}.")

    def delete_project(self, project_name: str):
        project_path = os.path.join(self.base_directory, project_name)
        if os.path.exists(project_path):
            shutil.rmtree(project_path)
            print(f"🗑️ Project '{project_name}' deleted.")
        else:
            print(f"⚠️ Project '{project_name}' does not exist.")

    def list_projects(self):
        return [d for d in os.listdir(self.base_directory) if os.path.isdir(os.path.join(self.base_directory, d))]

    def open_project(self, project_name: str):
        project_path = os.path.join(self.base_directory, project_name)
        if os.path.exists(project_path):
            print(f"📂 Opening project: {project_path}")
        else:
            print(f"⚠️ Project '{project_name}' does not exist.")

    def rename_project(self, old_name: str, new_name: str):
        old_path = os.path.join(self.base_directory, old_name)
        new_path = os.path.join(self.base_directory, new_name)
        if os.path.exists(old_path):
            os.rename(old_path, new_path)
            print(f"🔄 Project '{old_name}' renamed to '{new_name}'.")
        else:
            print(f"⚠️ Project '{old_name}' does not exist.")

    def delete_shader_file(self, project_name: str, file_name: str):
        file_path = os.path.join(self.base_directory, project_name, file_name)
        if os.path.exists(file_path):
            os.remove(file_path)
            print(f"🗑️ Shader file '{file_name}' deleted from '{project_name}'.")
        else:
            print(f"⚠️ Shader file '{file_name}' does not exist in '{project_name}'.")

    def list_shader_files(self, project_name: str):
        project_path = os.path.join(self.base_directory, project_name)
        if os.path.exists(project_path):
            return [f for f in os.listdir(project_path) if os.path.isfile(os.path.join(project_path, f))]
        return []

    def open_shader_file(self, project_name: str, file_name: str):
        file_path = os.path.join(self.base_directory, project_name, file_name)
        if os.path.exists(file_path):
            with open(file_path, 'r') as file:
                return file.read()
        print(f"⚠️ Shader file '{file_name}' does not exist in '{project_name}'.")
        return None

    def rename_shader_file(self, project_name: str, old_name: str, new_name: str):
        old_path = os.path.join(self.base_directory, project_name, old_name)
        new_path = os.path.join(self.base_directory, project_name, new_name)
        if os.path.exists(old_path):
            os.rename(old_path, new_path)
            print(f"🔄 Shader file '{old_name}' renamed to '{new_name}'.")
        else:
            print(f"⚠️ Shader file '{old_name}' does not exist in '{project_name}'.")

    def reset_project(self, project_name: str):
        project_path = os.path.join(self.base_directory, project_name)
        if os.path.exists(project_path):
            for file in os.listdir(project_path):
                file_path = os.path.join(project_path, file)
                if os.path.isfile(file_path):
                    os.remove(file_path)
            print(f"🔄 Project '{project_name}' reset. All shader files deleted.")
        else:
            print(f"⚠️ Project '{project_name}' does not exist.")
    def clear_base_directory(self):
        if os.path.exists(self.CONFIG_FILE):
            os.remove(self.CONFIG_FILE)
            print("✅ Base directory path cleared. You will be asked for a new path next time.")
        else:
            print("⚠️ No stored path found.")


pm = ProjectManager()
    

# project_name = "ayush"
# pm.create_project(project_name)
# print("Projects after creation:", pm.list_projects())
# pm.delete_project("TestProject")
# pm.clear_base_directory()