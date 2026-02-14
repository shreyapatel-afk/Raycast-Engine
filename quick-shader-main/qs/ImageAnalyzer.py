# ImageAnalyzer Functionality (10 Methods)
# get_brightness() → Calculates the average brightness of the image.
# get_contrast() → Measures contrast (difference between max & min pixel values).
# get_light_direction() → Estimates the dominant light direction using gradients.
# get_edges() → Detects edges using Canny edge detection.
# get_texture_complexity() → Measures texture complexity based on pixel intensity variance.
# get_histogram() → Generates a histogram of pixel intensity values.
# get_gradient_magnitude() → Computes the gradient magnitude for lighting analysis.
# get_average_color() → Returns the average grayscale value (acts as a "color").
# is_high_contrast(threshold=0.5) → Determines if the image has high contrast.
# analyze_image() → Returns all extracted properties in a dictionary.

import cv2
import numpy as np
import os

class ImageAnalyzer:
    def __init__(self):
            self.image_path = self.ask_for_image_path()
            self.image = self.load_image(self.image_path)

    def ask_for_image_path(self):
        while True:
            image_path = input("Enter image path: ").strip()

            # Automatically remove quotes if they exist
            if image_path.startswith('"') and image_path.endswith('"'):
                image_path = image_path[1:-1]

            if os.path.exists(image_path):
                return image_path

            print("❌ Invalid path. Please enter a valid image file path.")


    def load_image(self, image_path):
        
        image = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)
        if image is None:
            print("❌ Error: Failed to load image. Make sure it's a valid image file.")
            exit(1)
        print(f"✅ Image loaded successfully from {image_path}")
        return image

    def change_image_path(self):
        
        print("🔄 Changing image path...")
        self.image_path = self.ask_for_image_path()
        self.image = self.load_image(self.image_path)
        print(f"✅ New image loaded from {self.image_path}")


    def get_brightness(self):
        return np.mean(self.image)

    def get_contrast(self):
        return np.max(self.image) - np.min(self.image)

    def get_light_direction(self):
        sobelx = cv2.Sobel(self.image, cv2.CV_64F, 1, 0, ksize=5)
        sobely = cv2.Sobel(self.image, cv2.CV_64F, 0, 1, ksize=5)
        magnitude = np.sqrt(sobelx**2 + sobely**2)
        angle = np.arctan2(sobely, sobelx) * (180 / np.pi)
        return np.mean(angle)

    def get_edges(self):
        return cv2.Canny(self.image, 100, 200)

    def get_texture_complexity(self):
        laplacian = cv2.Laplacian(self.image, cv2.CV_64F)
        return laplacian.var()

    def get_histogram(self):
        hist = cv2.calcHist([self.image], [0], None, [256], [0, 256])
        return hist.flatten().tolist()

    def get_gradient_magnitude(self):
        sobelx = cv2.Sobel(self.image, cv2.CV_64F, 1, 0, ksize=5)
        sobely = cv2.Sobel(self.image, cv2.CV_64F, 0, 1, ksize=5)
        magnitude = np.sqrt(sobelx**2 + sobely**2)
        return np.mean(magnitude)

    def get_average_color(self):
        return np.mean(self.image)

    def is_high_contrast(self, threshold=0.5):
        contrast = self.get_contrast()
        normalized_contrast = contrast / 255
        return normalized_contrast > threshold

    def analyze_image(self):
        return {
            "brightness": self.get_brightness(),
            "contrast": self.get_contrast(),
            "light_direction": self.get_light_direction(),
            "texture_complexity": self.get_texture_complexity(),
            "gradient_magnitude": self.get_gradient_magnitude(),
            "average_color": self.get_average_color(),
            "is_high_contrast": self.is_high_contrast()
        }


# Create an instance of ImageAnalyzer
analyzer = ImageAnalyzer()

# Test all functions
print("\n🔍 Image Analysis Results:")
print("Brightness:", analyzer.get_brightness())
print("Contrast:", analyzer.get_contrast())
print("Light Direction:", analyzer.get_light_direction())
print("Texture Complexity:", analyzer.get_texture_complexity())
print("Gradient Magnitude:", analyzer.get_gradient_magnitude())
print("Average Color:", analyzer.get_average_color())
print("Is High Contrast:", analyzer.is_high_contrast())

# Test Histogram
histogram = analyzer.get_histogram()
print("\n📊 Histogram Sample (First 10 values):", histogram[:10])

# Test Edge Detection (Save and Open the Edge Image)
edges = analyzer.get_edges()
cv2.imwrite("edges_output.jpg", edges)  # Save the edge-detected image
print("\n✅ Edge-detected image saved as 'edges_output.jpg'. Open it manually to view.")

# Test Changing Image Path
print("\n🛠️ Testing Change Image Path Feature...")
analyzer.change_image_path()
print("New Path Set:", analyzer.image_path)
print("New Brightness:", analyzer.analyze_image()["brightness"])
