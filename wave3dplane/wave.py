import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

def plot_hanning_wave(size=100, depth=50, angle=70, cut_x=0):
    """Plots a 3D projection of a rotated Hanning window extended along the Z plane with an extended cutting plane and highlights the intersection, keeping Sample centered around 0."""
    
    # Define the sample size and depth (Sample will be centered around 0)
    x = np.linspace(-size // 2, size // 2, size)  # Center Sample around 0
    y = np.linspace(-depth // 2, depth // 2, depth)  # Center Depth around 0
    X, Y = np.meshgrid(x, y)
    Z = np.tile(np.hanning(size), (depth, 1))
    
    # Convert degrees to radians
    theta = np.radians(angle)
    
    # Apply rotation matrix
    X_rot = X * np.cos(theta) - Y * np.sin(theta)
    Y_rot = X * np.sin(theta) + Y * np.cos(theta)
    
    fig = plt.figure(figsize=(12, 6))
    
    # 3D Plot
    ax1 = fig.add_subplot(121, projection='3d')
    ax1.plot_surface(X_rot, Y_rot, Z, edgecolor='none', alpha=0.4)
    
    # Define the extended cutting plane
    cut_plane_y1 = np.linspace(Y_rot.min() - 10, Y_rot.max() + 10, depth)
    cut_plane_z1 = np.linspace(Z.min() - 0.1, Z.max() + 0.1, depth)
    Cut_Y1, Cut_Z1 = np.meshgrid(cut_plane_y1, cut_plane_z1)
    Cut_X1 = np.full_like(Cut_Y1, cut_x)

    ax1.plot_surface(Cut_X1, Cut_Y1, Cut_Z1, color='red', alpha=0.4)

    # Compute the actual intersection points
    intersection_mask = np.abs(X_rot - cut_x) < 1  # Tolerance for intersection
    intersection_x = np.full_like(Y_rot[intersection_mask], cut_x)
    intersection_y = Y_rot[intersection_mask]
    intersection_z = Z[intersection_mask]

    ax1.plot(intersection_x, intersection_y, intersection_z, color='red', linewidth=3, label='Intersection')
    
    
    ax1.set_xlabel("Sample")
    ax1.set_ylabel("Depth")
    ax1.set_zlabel("Amplitude")
    ax1.set_title("3D Rotated Hanning Window Wave with Extended Cutting Plane and Intersection")
    ax1.legend()
    
    # 2D Plot (Intersection Plot)
    ax2 = fig.add_subplot(122)
    
    # Project the intersection onto Depth vs Amplitude without oscillation
    ax2.plot(intersection_y, intersection_z, color='red', linewidth=3)
    
    ax2.set_xlabel("Depth")
    ax2.set_ylabel("Amplitude")
    ax2.set_title("2D Projection of Intersection of the Two Planes")
    
    # Keep the range fixed so that the intersection completes the entire up and down along Z
    ax2.set_ylim(np.min(intersection_z), np.max(intersection_z))
    
    # Ensure the x-axis range is centered around 0
    ax2.set_xlim([min(intersection_y.min(), 0), max(intersection_y.max(), 0)])
    
    # Calculate the width of the intersection at the current cut
    width = np.max(intersection_y) - np.min(intersection_y)
    ax2.text(0.05, 0.9, f"Width of Intersection: {width:.2f}", transform=ax2.transAxes, fontsize=12, color="blue")
    
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    plot_hanning_wave(1024, 1024*8, 45, 0)
