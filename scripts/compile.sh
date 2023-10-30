#!/bin/bash

# Define the source directory containing your shader files
SOURCE_DIR="../shaders"

# Define the output directory for the compiled SPIR-V files
OUTPUT_DIR="../bin/shaders"

# Create the output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Function to get the shader type from a shader source file
get_shader_type() {
  # Use grep to find a line in the file that specifies the shader type
  shader_type=$(grep -o -m 1 '^\s*//\s*type:\s*\w*' "$1" | awk -F'//' '{print $2}' | awk -F':' '{print $2}' | tr -d '[:space:]')
  echo "$shader_type"
}

# Find all .glsl files in the source directory and compile them
for shader_file in "$SOURCE_DIR"/*.glsl; do
  if [ -f "$shader_file" ]; then
    # Extract the filename (without extension) from the full path
    filename=$(basename "$shader_file")
    filename_no_ext="${filename%.*}"

    # Get the shader type from the source file
    shader_type=$(get_shader_type "$shader_file")

    if [ -n "$shader_type" ]; then
      # Compile the shader to SPIR-V using glslc, specifying the shader type

      glslc -fshader-stage="${shader_type}" --target-env=vulkan1.2 --target-spv=spv1.3  "$shader_file" -o "$OUTPUT_DIR/$filename_no_ext.spv"

      # Check if the compilation was successful
      if [ $? -eq 0 ]; then
        echo "Compiled $shader_file to $OUTPUT_DIR/$filename_no_ext.spv (Type: $shader_type)"
      else
        echo "Failed to compile $shader_file"
        exit 1
      fi
    else
      echo "Error: Shader type not specified in $shader_file"
      exit 1
    fi
  fi
done

echo "Shader compilation complete."

# Optionally, you can add additional steps here, like linking the SPIR-V files into a Vulkan pipeline.
