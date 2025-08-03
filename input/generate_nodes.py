import os
import random

def main():
    # Ask the user for the boundary size
    try:
        boundary_size = int(input("Enter boundary size (integer): "))
    except ValueError:
        print("Invalid input. Please enter an integer.")
        return

    # Determine node count
    try:
        node_count = int(input("Enter node count (integer): "))
    except ValueError:
        print("Invalid input. Please enter an integer.")
        return

    filename = f"genNodes{boundary_size}_{node_count}.txt"

    # Ensure output is saved in the same directory as the script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    filepath = os.path.join(script_dir, filename)

    # Generate unique random nodes using a set
    nodes = set()
    while len(nodes) < node_count:
        x = random.randint(0, boundary_size)
        y = random.randint(0, boundary_size)
        nodes.add((x, y))

    # Write to the output file
    with open(filepath, "w") as f:
        f.write(f"0 0 {boundary_size} {boundary_size}\n")
        f.write(f"{node_count}\n")
        for x, y in nodes:
            f.write(f"{x} {y}\n")

    print(f"Generated {node_count} unique nodes in '{filepath}'.")

if __name__ == "__main__":
    main()
