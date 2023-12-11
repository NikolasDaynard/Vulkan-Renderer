#version 450

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    const int levels = 10; // Adjust the number of levels as needed

    // Quantize the color values
    vec3 posterizedColor = floor(fragColor * float(levels)) / float(levels);
    vec3 difference = fragColor - posterizedColor;
    float differenceValue = ((difference.r + difference.g + difference.b) / 3);
    if(differenceValue > float(levels) / 250){
        outColor = vec4(fragColor, 1.0);
    }else{
        outColor = vec4(posterizedColor, 1.0);
    }
    // Output the posterized color


    //outColor = vec4(fragColor, 1.0);
}
