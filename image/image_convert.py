from PIL import Image
import numpy as np

# Load and convert image to RGBA
img = Image.open("KeblaOS_icon_320x200x32.png").convert("RGBA")
data = np.array(img)

# Save the raw RGBA values to a file
data.flatten().tofile("image_data.bin")

