"""
Map Pixel Coordinate Clicker  —  with Object/Group support + Zoom & Pan
========================================================================
1. Type an object name in the box (e.g. "Hole 1") and press Enter or click Set
2. LEFT CLICK on the map  → records a point under that object
3. RIGHT CLICK            → undo last point
4. Scroll wheel           → zoom in / out (centred on cursor)
5. Middle-click + drag    → pan around the map
6. Press  Z / X           → zoom in / out with keyboard
7. Press  R               → reset zoom & pan to fit screen
8. Press  S               → save labeled PNG + CSV grouped by object
9. Press  C               → clear ALL points
10. Press Q               → quit

Each object gets its own colour. Coordinates are always saved in
original image pixels regardless of zoom level.
"""

import tkinter as tk
from tkinter import filedialog, messagebox, simpledialog
from PIL import Image, ImageDraw, ImageFont, ImageTk
import csv, os, sys

# ── Config ────────────────────────────────────────────────────────────────────
DOT_RADIUS   = 5
FONT_SIZE    = 12
TEXT_BG      = "white"
OUTPUT_DIR   = os.path.dirname(os.path.abspath(__file__))
ZOOM_STEP    = 1.2          # multiply/divide zoom by this per scroll tick
ZOOM_MIN     = 0.1
ZOOM_MAX     = 10.0

PALETTE = [
    "#e6194b","#3cb44b","#4363d8","#f58231","#911eb4",
    "#42d4f4","#f032e6","#bfef45","#fabed4","#469990",
    "#dcbeff","#9A6324","#fffac8","#800000","#aaffc3",
    "#808000","#ffd8b1","#000075","#a9a9a9","#dddddd",
]
# ──────────────────────────────────────────────────────────────────────────────

# Canvas display size (window will not exceed this)
CANVAS_W = 1100
CANVAS_H = 750


class MapClicker:
    def __init__(self, root: tk.Tk, image_path: str):
        self.root       = root
        self.image_path = image_path

        # Data store
        self.objects: dict[str, list[tuple[int,int]]] = {}
        self.obj_order: list[str] = []
        self.current_obj: str = ""
        self._history: list[tuple[str,int,int]] = []

        # Load original image
        self.orig = Image.open(image_path).convert("RGBA")
        self.img_w, self.img_h = self.orig.size

        # Zoom / pan state
        fit_zoom = min(CANVAS_W / self.img_w, CANVAS_H / self.img_h)
        self.zoom   = fit_zoom          # current scale factor
        self.offset = [0, 0]           # canvas offset (pan) in pixels
        self._pan_start = None         # for middle-click drag

        # ── Layout ──────────────────────────────────────────────────────────
        root.title("Map Clicker  |  Scroll=zoom  MiddleDrag=pan  R=reset  S=save  Q=quit")

        # Top toolbar
        toolbar = tk.Frame(root, pady=4, padx=6)
        toolbar.pack(fill=tk.X)

        tk.Label(toolbar, text="Active object:").pack(side=tk.LEFT)
        self.obj_var = tk.StringVar()
        self.obj_entry = tk.Entry(toolbar, textvariable=self.obj_var, width=22, font=("Arial", 11))
        self.obj_entry.pack(side=tk.LEFT, padx=(4,2))
        self.obj_entry.bind("<Return>", self._set_object)

        tk.Button(toolbar, text="Set  ✔", command=self._set_object,
                  bg="#3cb44b", fg="white", font=("Arial", 10, "bold")).pack(side=tk.LEFT, padx=2)

        self.active_label = tk.Label(toolbar, text="No object selected",
                                     font=("Arial", 10, "bold"), fg="gray")
        self.active_label.pack(side=tk.LEFT, padx=10)

        # Zoom buttons in toolbar
        tk.Label(toolbar, text="   Zoom:").pack(side=tk.LEFT)
        tk.Button(toolbar, text="  +  ", command=lambda: self._zoom_by(ZOOM_STEP),
                  font=("Arial", 11, "bold")).pack(side=tk.LEFT, padx=1)
        tk.Button(toolbar, text="  −  ", command=lambda: self._zoom_by(1/ZOOM_STEP),
                  font=("Arial", 11, "bold")).pack(side=tk.LEFT, padx=1)
        tk.Button(toolbar, text=" Reset ", command=self._reset_view,
                  font=("Arial", 10)).pack(side=tk.LEFT, padx=4)
        self.zoom_label = tk.Label(toolbar, text="100%", font=("Arial", 10), width=6)
        self.zoom_label.pack(side=tk.LEFT)

        # Side panel
        side = tk.Frame(root)
        side.pack(side=tk.RIGHT, fill=tk.Y, padx=4, pady=4)
        tk.Label(side, text="Objects", font=("Arial", 10, "bold")).pack()
        self.obj_listbox = tk.Listbox(side, width=22, font=("Arial", 9))
        self.obj_listbox.pack(fill=tk.BOTH, expand=True)
        self.obj_listbox.bind("<<ListboxSelect>>", self._select_from_list)

        # Canvas with scrollbars
        canvas_frame = tk.Frame(root)
        canvas_frame.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        self.canvas = tk.Canvas(canvas_frame, width=CANVAS_W, height=CANVAS_H,
                                bg="#2b2b2b", cursor="crosshair")
        self.canvas.pack(fill=tk.BOTH, expand=True)

        # Status bar
        self.status = tk.StringVar(value="Type an object name above and press Enter, then click the map.")
        tk.Label(root, textvariable=self.status, anchor="w",
                 relief=tk.SUNKEN, bd=1).pack(fill=tk.X, side=tk.BOTTOM)

        # Bindings
        self.canvas.bind("<Button-1>",        self._on_left_click)
        self.canvas.bind("<Button-3>",        self._on_right_click)
        self.canvas.bind("<Motion>",          self._on_mouse_move)
        # Scroll zoom
        self.canvas.bind("<MouseWheel>",      self._on_scroll)       # Windows
        self.canvas.bind("<Button-4>",        self._on_scroll)       # Linux up
        self.canvas.bind("<Button-5>",        self._on_scroll)       # Linux down
        # Middle-click pan
        self.canvas.bind("<Button-2>",        self._pan_start_fn)
        self.canvas.bind("<B2-Motion>",       self._pan_move)
        self.canvas.bind("<ButtonRelease-2>", self._pan_end)

        root.bind("<z>", lambda e: self._zoom_by(ZOOM_STEP))
        root.bind("<Z>", lambda e: self._zoom_by(ZOOM_STEP))
        root.bind("<x>", lambda e: self._zoom_by(1/ZOOM_STEP))
        root.bind("<X>", lambda e: self._zoom_by(1/ZOOM_STEP))
        root.bind("<r>", lambda e: self._reset_view())
        root.bind("<R>", lambda e: self._reset_view())
        root.bind("<Left>",  lambda e: self._pan_by( 40,   0))
        root.bind("<Right>", lambda e: self._pan_by(-40,   0))
        root.bind("<Up>",    lambda e: self._pan_by(  0,  40))
        root.bind("<Down>",  lambda e: self._pan_by(  0, -40))
        root.bind("<s>", self._save); root.bind("<S>", self._save)
        root.bind("<C>", self._clear)  # Shift+C to clear
        root.bind("<q>", lambda e: root.destroy()); root.bind("<Q>", lambda e: root.destroy())

        self._reset_view()

    # ── Coordinate helpers ───────────────────────────────────────────────────

    def _canvas_to_image(self, cx, cy):
        """Convert canvas pixel → original image pixel."""
        ix = (cx - self.offset[0]) / self.zoom
        iy = (cy - self.offset[1]) / self.zoom
        return int(ix), int(iy)

    def _image_to_canvas(self, ix, iy):
        """Convert original image pixel → canvas pixel."""
        cx = ix * self.zoom + self.offset[0]
        cy = iy * self.zoom + self.offset[1]
        return cx, cy

    # ── Zoom / pan ───────────────────────────────────────────────────────────

    def _zoom_by(self, factor, cx=None, cy=None):
        """Zoom by factor, keeping canvas point (cx,cy) fixed."""
        new_zoom = max(ZOOM_MIN, min(ZOOM_MAX, self.zoom * factor))
        if new_zoom == self.zoom:
            return
        if cx is None:
            cx = CANVAS_W / 2
        if cy is None:
            cy = CANVAS_H / 2
        # Adjust offset so the image point under cursor stays put
        self.offset[0] = cx - (cx - self.offset[0]) * (new_zoom / self.zoom)
        self.offset[1] = cy - (cy - self.offset[1]) * (new_zoom / self.zoom)
        self.zoom = new_zoom
        self.zoom_label.config(text=f"{int(self.zoom*100)}%")
        self._redraw()

    def _reset_view(self):
        fit = min(CANVAS_W / self.img_w, CANVAS_H / self.img_h)
        self.zoom = fit
        self.offset = [
            (CANVAS_W - self.img_w * fit) / 2,
            (CANVAS_H - self.img_h * fit) / 2,
        ]
        self.zoom_label.config(text=f"{int(self.zoom*100)}%")
        self._redraw()

    def _on_scroll(self, event):
        # Windows: event.delta; Linux: Button-4/5
        if event.num == 4 or event.delta > 0:
            self._zoom_by(ZOOM_STEP, event.x, event.y)
        else:
            self._zoom_by(1/ZOOM_STEP, event.x, event.y)

    def _pan_start_fn(self, event):
        self._pan_start = (event.x, event.y, self.offset[0], self.offset[1])
        self.canvas.config(cursor="fleur")

    def _pan_move(self, event):
        if self._pan_start:
            sx, sy, ox, oy = self._pan_start
            self.offset[0] = ox + (event.x - sx)
            self.offset[1] = oy + (event.y - sy)
            self._redraw()

    def _pan_end(self, event):
        self._pan_start = None
        self.canvas.config(cursor="crosshair")

    def _pan_by(self, dx, dy):
        """Shift the view by dx, dy canvas pixels (used by arrow keys)."""
        self.offset[0] += dx
        self.offset[1] += dy
        self._redraw()

    # ── Helpers ──────────────────────────────────────────────────────────────

    def _color_for(self, obj_name):
        if obj_name not in self.obj_order:
            return "#ff0000"
        return PALETTE[self.obj_order.index(obj_name) % len(PALETTE)]

    def _total_points(self):
        return sum(len(v) for v in self.objects.values())

    # ── Object management ────────────────────────────────────────────────────

    def _set_object(self, event=None):
        name = self.obj_var.get().strip()
        if not name:
            messagebox.showwarning("No name", "Please type an object name first.")
            return
        self.current_obj = name
        if name not in self.objects:
            self.objects[name] = []
            self.obj_order.append(name)
            self.obj_listbox.insert(tk.END, name)
            self.obj_listbox.itemconfig(tk.END, fg=self._color_for(name))
        idx = self.obj_order.index(name)
        self.obj_listbox.selection_clear(0, tk.END)
        self.obj_listbox.selection_set(idx)
        self.obj_listbox.see(idx)
        self.active_label.config(text=f"● {name}", fg=self._color_for(name))
        self.status.set(f"Active object: {name}  |  Now click on the map to record points.")
        self.obj_entry.delete(0, tk.END)

    def _select_from_list(self, event=None):
        sel = self.obj_listbox.curselection()
        if sel:
            name = self.obj_order[sel[0]]
            self.current_obj = name
            self.active_label.config(text=f"● {name}", fg=self._color_for(name))
            self.status.set(f"Switched to: {name}  ({len(self.objects.get(name,[]))} points)")

    # ── Click handlers ────────────────────────────────────────────────────────

    def _on_left_click(self, event):
        if not self.current_obj:
            messagebox.showinfo("Set an object first",
                                "Type an object name at the top and press Enter before clicking.")
            return
        ix, iy = self._canvas_to_image(event.x, event.y)
        # Clamp to image bounds
        ix = max(0, min(self.img_w - 1, ix))
        iy = max(0, min(self.img_h - 1, iy))
        self.objects[self.current_obj].append((ix, iy))
        self._history.append((self.current_obj, ix, iy))
        pts = len(self.objects[self.current_obj])
        self.status.set(
            f"[{self.current_obj}] point #{pts} → {ix},{iy}px  "
            f"(zoom {int(self.zoom*100)}%  |  total: {self._total_points()})"
        )
        self._redraw()

    def _on_right_click(self, event=None):
        if not self._history:
            return
        obj, x, y = self._history.pop()
        if obj in self.objects and self.objects[obj]:
            self.objects[obj].pop()
        self.status.set(f"Undo: removed ({x},{y}) from [{obj}]")
        self._redraw()

    def _on_mouse_move(self, event):
        ix, iy = self._canvas_to_image(event.x, event.y)
        obj_info = f"Active: {self.current_obj}" if self.current_obj else "No object set"
        self.status.set(
            f"Image coords: {ix},{iy}px  |  Zoom: {int(self.zoom*100)}%  |  "
            f"{obj_info}  |  Total pts: {self._total_points()}"
        )

    # ── Drawing ───────────────────────────────────────────────────────────────

    def _redraw(self):
        # Scale the original image to current zoom
        disp_w = max(1, int(self.img_w * self.zoom))
        disp_h = max(1, int(self.img_h * self.zoom))
        scaled = self.orig.resize((disp_w, disp_h), Image.NEAREST if self.zoom > 3 else Image.LANCZOS)

        composite = scaled.copy()
        draw = ImageDraw.Draw(composite)

        try:
            font = ImageFont.truetype(
                "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", FONT_SIZE)
        except OSError:
            font = ImageFont.load_default()

        for obj_name in self.obj_order:
            color = self._color_for(obj_name)
            for pt_idx, (ix, iy) in enumerate(self.objects.get(obj_name, []), 1):
                # Convert image coords → scaled image coords
                sx = int(ix * self.zoom)
                sy = int(iy * self.zoom)
                num_str = str(pt_idx)
                r = max(DOT_RADIUS, int(DOT_RADIUS + 3))
                draw.ellipse([sx-r, sy-r, sx+r, sy+r], fill=color, outline="white", width=1)
                bbox = draw.textbbox((0, 0), num_str, font=font)
                tw, th = bbox[2]-bbox[0], bbox[3]-bbox[1]
                draw.text((sx - tw//2, sy - th//2), num_str, fill="white", font=font)

        self._tk_img = ImageTk.PhotoImage(composite.convert("RGB"))
        self.canvas.delete("all")
        self.canvas.create_image(int(self.offset[0]), int(self.offset[1]),
                                 anchor="nw", image=self._tk_img)

    # ── Save ─────────────────────────────────────────────────────────────────

    def _save(self, event=None):
        if self._total_points() == 0:
            messagebox.showinfo("Nothing to save", "No points recorded yet.")
            return

        base = os.path.splitext(os.path.basename(self.image_path))[0]

        # Ask where to save CSV
        csv_out = filedialog.asksaveasfilename(
            title="Save coordinates CSV as...",
            initialdir=OUTPUT_DIR,
            initialfile=f"{base}_coords.csv",
            defaultextension=".csv",
            filetypes=[("CSV files", "*.csv"), ("All files", "*.*")]
        )
        if not csv_out:
            return  # user cancelled

        # Ask where to save labeled image
        img_out = filedialog.asksaveasfilename(
            title="Save labeled image as...",
            initialdir=os.path.dirname(csv_out),
            initialfile=f"{base}_labeled.png",
            defaultextension=".png",
            filetypes=[("PNG files", "*.png"), ("All files", "*.*")]
        )
        if not img_out:
            return  # user cancelled

        # Draw labeled image at ORIGINAL resolution
        labeled = self.orig.copy().convert("RGB")
        draw = ImageDraw.Draw(labeled)
        try:
            font = ImageFont.truetype(
                "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", FONT_SIZE)
        except OSError:
            font = ImageFont.load_default()

        for obj_name in self.obj_order:
            color = self._color_for(obj_name)
            for pt_idx, (x, y) in enumerate(self.objects[obj_name], 1):
                num_str = str(pt_idx)
                r = DOT_RADIUS + 3
                draw.ellipse([x-r, y-r, x+r, y+r], fill=color, outline="white", width=1)
                bbox = draw.textbbox((0, 0), num_str, font=font)
                tw, th = bbox[2]-bbox[0], bbox[3]-bbox[1]
                draw.text((x - tw//2, y - th//2), num_str, fill="white", font=font)
        labeled.save(img_out)

        # Save CSV
        with open(csv_out, "w", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(["object", "point_#", "x", "y"])
            for obj_name in self.obj_order:
                for pt_idx, (x, y) in enumerate(self.objects[obj_name], 1):
                    writer.writerow([obj_name, pt_idx, x, y])

        messagebox.showinfo(
            "Saved!",
            f"Labeled image  →  {img_out}\n"
            f"Coordinates CSV →  {csv_out}\n\n"
            f"{len(self.obj_order)} objects, {self._total_points()} points total."
        )

    # ── Clear ────────────────────────────────────────────────────────────────

    def _clear(self, event=None):
        if messagebox.askyesno("Clear all?", "Remove ALL objects and points?"):
            self.objects.clear()
            self.obj_order.clear()
            self._history.clear()
            self.current_obj = ""
            self.active_label.config(text="No object selected", fg="gray")
            self.obj_listbox.delete(0, tk.END)
            self.status.set("Cleared.")
            self._redraw()


# ── Entry point ───────────────────────────────────────────────────────────────

def main():
    if len(sys.argv) > 1:
        image_path = sys.argv[1]
    else:
        root_tmp = tk.Tk(); root_tmp.withdraw()
        image_path = filedialog.askopenfilename(
            title="Select your map image",
            filetypes=[("Images", "*.png *.jpg *.jpeg *.bmp *.tif *.tiff"),
                       ("All files", "*.*")]
        )
        root_tmp.destroy()
        if not image_path:
            print("No file selected. Exiting.")
            sys.exit(0)

    root = tk.Tk()
    MapClicker(root, image_path)
    root.mainloop()


if __name__ == "__main__":
    main()