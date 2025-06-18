import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os
from math import floor

# Load the data
df = pd.read_csv("~/Downloads/zw_weeks.csv")

# Calculate Resistance
df["Resistance"] = 1 / (df["Conductivity"] * 100)

# Round the Time to the nearest hour
df["Time"] = pd.to_datetime(df["Time"]).dt.round("1h")

# Create output directory if it doesn't exist
output_dir = "out"
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

# Show the first few rows of the DataFrame
print(df.head())

# Calculate the number of plots to create
count = floor(len(df) / 23)

# Set the x-ticks
x_ticks = [-16.74, -16.99, -17.29, -17.59, -17.89, -18.19, -18.49, -18.79, -19.09, -19.39,
           -19.69, -19.99, -20.29, -20.59, -20.89, -21.19, -21.49, -21.79, -22.09, -22.39, -22.69]

# Loop through each chunk and create a plot
for i in range(count):
    sdf = df.iloc[(i * 23)+1:(i * 23) + 22]
    fig, ax = plt.subplots()
    ax.set_title(sdf.iloc[0]["Time"])
    ax.set_xlabel("Depth (mNAP)")
    ax.set_ylabel("Resistance (ohm)")
    ax.set_ylim((0, 3))
    ax.set_xlim((-23, -16))  # Correct the x-axis limits to match the ticks
    ax.set_xticks(x_ticks)
    for tick in ax.get_xticklabels():
        tick.set_rotation(75)
    ax.invert_xaxis()  # Invert the x-axis
    ax.grid()
    ax.plot(x_ticks, sdf["Resistance"])
    fig.tight_layout()
    fig.savefig(f"{output_dir}/{i}.png")
    plt.close(fig)  # Close the figure to avoid memory issues

print(f"Plots saved in the '{output_dir}' directory.")
