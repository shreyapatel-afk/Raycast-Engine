# __init__(self, shader_name="GeneratedShader") → Initializes the shader generator.

# generate_shader_code(self, extracted_data: dict) → Generates shader code using extracted data.

# save_shader(self, output_path="GeneratedShader.shader") → Saves the generated shader file.

# get_shader_code(self) → Returns the current shader code as a string.

# apply_modifications(self, modified_data: dict) → Applies modified brightness, contrast, etc.

# enable_texture_mapping(self, enable: bool) → Enables/disables texture mapping.

# reset_settings(self) → Resets shader attributes to default.

# save_checkpoint(self) → Saves the current shader settings as a checkpoint.

# undo(self) → Restores the previous shader checkpoint.

# redo(self) → Restores a checkpoint that was undone.