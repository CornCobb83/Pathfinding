import pathfinder
import tkinter as tk

tile_size = 15
fileName = "map.txt"

# Read map
map_data, map_length, map_height = pathfinder.read_map(fileName)

if (map_data) and map_length != 0 and map_height != 0:

    root = tk.Tk()
    root.title("Interactive Pathfinding")

    canvas_width = map_length * tile_size
    canvas_height = map_height * tile_size

    # Scrollable canvas in case map is larger than screen
    canvas = tk.Canvas(root, width=canvas_width,
                       height=canvas_height,
                       scrollregion=(0, 0, canvas_width, canvas_height))
    canvas.pack(fill=tk.BOTH, expand=True)

    # Start and end points (row, col)
    start, end = None, None

    # Create grid of rectangles
    tiles = [[None for _ in range(map_length)] for _ in range(map_height)]
    original_colors = [[None for _ in range(map_length)] for _ in range(map_height)]

    # Color mapping
    color_map = {
        "f": "green",
        "M": "brown",
        "~": "blue",
        "_": "white"
    }

    # Update start and end tile colors
    def update_start_end():
        if start:
            r, c = start
            canvas.itemconfig(tiles[r][c], fill="lightgreen")
        if end:
            r, c = end
            canvas.itemconfig(tiles[r][c], outline="red", width=3)

    # Display the computed path
    def compute_display(path):
        # Reset all tiles
        for i in range(map_height):
            for j in range(map_length):
                canvas.itemconfig(tiles[i][j],
                                  fill=original_colors[i][j],
                                  outline="black", width=1)
        # Draw path
        for (r, c) in path:
            if (r, c) != start and (r, c) != end:
                canvas.itemconfig(tiles[r][c], outline="orange", width=5)
        update_start_end()

    def clear_path():
        for i in range(map_height):
            for j in range(map_length):
                try:
                    canvas.itemconfig(tiles[i][j], fill=original_colors[i][j], outline="black", width=1)
                except:
                    pass
        update_start_end()

    # Handle tile click
    def tile_clicked(row, col):
        global start, end
        coords = (row, col)

        if coords == start:
            start = None
            clear_path()
        elif coords == end:
            end = None
            clear_path()
        elif start is None:
            start = coords
            update_start_end()
        elif end is None:
            end = coords
            update_start_end()

        if start and end:
            try:
                path = pathfinder.find_path(fileName, start[0], start[1], end[0], end[1])
                compute_display(path)
            except:
                pass 

    # --- Draw Map ---

    for i, row in enumerate(map_data):
        for j, tile in enumerate(row):
            color = color_map.get(tile, "white")
            x1, y1 = j * tile_size, i * tile_size
            x2, y2 = x1 + tile_size, y1 + tile_size

            rect_id = canvas.create_rectangle(x1, y1, x2, y2, fill=color, outline="black")
            tiles[i][j] = rect_id
            original_colors[i][j] = color

            if tile != "~":  # allow clicking on non-water tiles
                canvas.tag_bind(rect_id, "<Button-1>", lambda e, row=i, col=j: tile_clicked(row, col))

    root.mainloop()

else:
    print("Failed to read map from file or map is empty.")
    print("Quitting...")
