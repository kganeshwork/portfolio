"""
Run this on Google Colab or your PC to generate CIFAR-10 test images as C arrays
for embedding in your STM32 project.
"""
import tensorflow as tf
import numpy as np

(_, _), (x_test, y_test) = tf.keras.datasets.cifar10.load_data()
y_test = y_test.flatten()

labels = ["airplane","automobile","bird","cat","deer",
          "dog","frog","horse","ship","truck"]

lines = ["#pragma once", "#include <stdint.h>", ""]
lines.append(f"#define NUM_TEST_IMAGES 10")
lines.append(f"#define IMAGE_SIZE_BYTES 3072  // 32*32*3")
lines.append("")

for cls in range(10):
    idx = np.where(y_test == cls)[0][0]
    img = x_test[idx].flatten()
    arr = "{" + ",".join(str(v) for v in img) + "}"
    lines.append(f"// Class {cls}: {labels[cls]}")
    lines.append(f"static const uint8_t test_img_{cls}[IMAGE_SIZE_BYTES] = {arr};")
    lines.append("")

lines.append("static const uint8_t* test_images[NUM_TEST_IMAGES] = {")
for cls in range(10):
    lines.append(f"    test_img_{cls},  // {labels[cls]}")
lines.append("};")
lines.append("")
lines.append("static const int test_labels[NUM_TEST_IMAGES] = {0,1,2,3,4,5,6,7,8,9};")
lines.append(f'static const char* class_labels[10] = {{"{chr(34).join(labels)}"}}; // wrong join, fix below')

# Fix the labels line properly
lines[-1] = 'static const char* class_labels[10] = {"' + '","'.join(labels) + '"};'

with open("test_images.h", "w") as f:
    f.write("\n".join(lines))

print("Generated test_images.h")