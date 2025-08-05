import re
import os


def extract_uniforms_from_glsl(glsl_file_path):
    """Extracts uniform variables from a GLSL file and generates a C++ UniformMap."""
    # Read the GLSL file
    with open(glsl_file_path, 'r') as file:
        glsl_code = file.read()

    # Regular expression to match uniform declarations
    uniform_pattern = r"uniform\s+(?P<type>\w+)\s+(?P<name>\w+)\s*;"
    matches = re.finditer(uniform_pattern, glsl_code)

    # Prepare the output lines
    output_lines = []
    for match in matches:
        var_name = match.group('name')
        output_lines.append(f'    {{"{var_name}", {var_name}}},')

    # Determine output file path
    output_path = os.path.splitext(glsl_file_path)[0] + '_uniforms.json'

    # Write to output file
    with open(output_path, 'w') as file:
        file.write("Shader::UniformMap u_ = {\n")
        file.write("\n".join(output_lines))
        file.write("\n};\n")

    print(f"Uniform map generated at: {output_path}")


if __name__ == "__main__":
    import sys

    if len(sys.argv) != 2:
        print("Usage: python parser.py <path_to_glsl_file>")
        sys.exit(1)

    glsl_file = sys.argv[1]
    if not os.path.isfile(glsl_file):
        print(f"Error: File not found - {glsl_file}")
        sys.exit(1)

    extract_uniforms_from_glsl(glsl_file)