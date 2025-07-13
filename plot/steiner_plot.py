"""Steiner tree plotter."""
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from typing import Any, Dict, Tuple
import argparse
import copy
import math
import cairo


class SteinerPlot:
    """A class to visualize a Steiner tree using Cairo."""

    # Default name of the png file to save the plot.
    DEFAULT_PNG_NAME = 'steiner.png'

    def __init__(self) -> None:
        """Initialize the plotter."""
        self.data = {}

        self.params: Dict[str, Any] = {
            # Plot margin ratio x.
            # This is calculated as the ratio of the boundary width.
            'plot_margin_ratio_x': 0.1,
            # Plot margin ratio y.
            # This is calculated as the ratio of the boundary height.
            'plot_margin_ratio_y': 0.1,
            # Boundary line width.
            'boundary_linewidth': 1.0,
            # Boundary line RGBA.
            'boundary_rgba': (0.9, 0.9, 0.9, 0.5),
            # Node RGBA.
            'node_rgba': (0, 0, 0, 1),
            # Edge RGBA.
            'edge_rgba': (0, 0, 1, 0.4),
            # Edge line width.
            'edge_linewidth': 5.0,
            # Min surface width.
            'min_surface_width': 4000,
            # Min surface height.
            'min_surface_height': 4000,
            # Surface factor.
            # Boundary width/height * surface factor = surface width/height.
            'surface_factor': 10,
        }

    def read_input(self, filename: str) -> None:
        """Read the input file.

        This is a sample implementation. No error checking is done.

        Args:
            filename(str): The path to the input file.
        """
        with open(filename, 'r', encoding='utf-8') as f:
            lines = [line.strip() for line in f.readlines() if line.strip()]

            # Parse boundary.
            min_x, min_y, max_x, max_y = map(int, lines[0].split())

            # Store the boundary.
            self.data['boundary'] = (min_x, min_y, max_x, max_y)

            # Parse nodes.
            num_nodes = int(lines[1])
            nodes = []
            for line in lines[2:]:
                x, y = map(int, line.split())
                nodes.append((x, y))

            assert len(nodes) == num_nodes

            # Store the nodes.
            self.data['nodes'] = copy.deepcopy(nodes)

    def read_output(self, filename: str) -> bool:
        """Read the input file.

        This is a sample implementation. No error checking is done.

        Args:
            filename(str): The path to the input file.
        """
        with open(filename, 'r', encoding='utf-8') as f:
            lines = [line.strip() for line in f.readlines() if line.strip()]

            # Parse edges.
            num_edges = int(lines[0])

            edges = []
            for line in lines[1:]:
                ux, uy, vx, vy = map(int, line.split())
                edges.append(((ux, uy), (vx, vy)))

            assert len(edges) == num_edges

            self.data['edges'] = copy.deepcopy(edges)

    def _get_boundary(self) -> Tuple[int, int, int, int]:
        """Gets the boundary.

        Returns:
            A tuple of(x_low, y_low, x_high, y_high)
        """
        if not self.data['boundary']:
            return (0, 0, 0, 0)
        return tuple(self.data['boundary'])

    def _get_boundary_size(self) -> Tuple[int, int]:
        """Gets the width and height of the boundary.

        Returns:
            A tuple of (width, height)
        """
        x_low, y_low, x_high, y_high = self._get_boundary()
        width, height = x_high - x_low, y_high - y_low

        return width, height

    def _get_plot_boundary(self) -> Tuple[int, int, int, int]:
        """Gets the boundary of the plot.

        This method calculates the boundary of the plot based on the boundary
        and the plot margin.

        Returns:
            A tuple of(x_low, y_low, x_high, y_high)
        """
        if not self.data['boundary']:
            return (0, 0, 0, 0)

        plot_margin_ratio_x = self.params['plot_margin_ratio_x']
        plot_margin_ratio_y = self.params['plot_margin_ratio_y']
        bdry_xl, bdry_yl, bdry_xh, bdry_yh = self._get_boundary()

        plot_margin_x = (bdry_xh - bdry_xl) * plot_margin_ratio_x
        plot_margin_y = (bdry_yh - bdry_yl) * plot_margin_ratio_y

        plot_boundary = (bdry_xl - plot_margin_x, bdry_yl - plot_margin_y,
                         bdry_xh + plot_margin_x, bdry_yh + plot_margin_y)

        return plot_boundary

    def _get_plot_boundary_size(self) -> Tuple[int, int]:
        """Gets the width and height of the plot boundary.

        Returns:
            A tuple of (width, height)
        """
        x_low, y_low, x_high, y_high = self._get_plot_boundary()
        width, height = x_high - x_low, y_high - y_low

        return width, height

    def _get_surface_size(self) -> Tuple[int, int]:
        """Get the size of the surface.

        Returns:
            A tuple of (width, height)
        """
        bdry_width, bdry_height = self._get_boundary_size()

        surface_width = max(
            self.params['min_surface_width'],
            bdry_width * self.params['surface_factor'])
        surface_height = max(
            self.params['min_surface_height'],
            bdry_height * self.params['surface_factor'])

        return surface_width, surface_height

    def _adjust_linewidth(self, scale_factor: float) -> None:
        """Adjusts the linewidth based on the scale factor.

        Args:
            scale_factor: The scale factor used to adjust the linewidth.
        """
        self.params['boundary_linewidth'] /= scale_factor
        self.params['edge_linewidth'] /= scale_factor

    def _draw_boundary(self, context: cairo.Context) -> None:
        """Draw the boundary of the plot.

        Args:
            context: The cairo context.
        """
        bdry_xl, bdry_yl, bdry_xh, bdry_yh = self._get_boundary()
        bdry_width, bdry_height = bdry_xh - bdry_xl, bdry_yh - bdry_yl

        context.set_source_rgba(*self.params['boundary_rgba'])
        context.rectangle(bdry_xl, bdry_yl, bdry_width, bdry_height)
        context.fill()

    def _draw_nodes(self, context: cairo.Context) -> None:
        """Draw the nodes.

        Args:
            context: The cairo context.
        """
        bdry_width, bdry_height = self._get_boundary_size()
        radius = math.log2(min(bdry_width, bdry_height)) * 0.025

        for node in self.data['nodes']:
            x, y = node
            context.set_source_rgba(*self.params['node_rgba'])
            context.arc(x, y, radius, 0, 2 * math.pi)
            context.fill()

    def _draw_edges(self, context: cairo.Context) -> None:
        """Draw the edges.

        Args:
            context: The cairo context.
        """
        bdry_width, bdry_height = self._get_boundary_size()
        radius = math.log2(math.sqrt(bdry_width * bdry_height)) * 0.01

        for edge in self.data['edges']:
            (ux, uy), (vx, vy) = edge

            # Endpoint (ux, uy)
            context.set_source_rgba(*self.params['edge_rgba'])
            context.arc(ux, uy, radius, 0, 2 * math.pi)
            context.fill()

            # Endpoint (vx, vy)
            context.set_source_rgba(*self.params['edge_rgba'])
            context.arc(vx, vy, radius, 0, 2 * math.pi)
            context.fill()

            # Edge (ux, uy) -> (vx, vy)
            context.set_source_rgba(*self.params['edge_rgba'])
            context.set_line_width(self.params['edge_linewidth'])
            context.move_to(ux, uy)
            context.line_to(vx, vy)
            context.stroke()

    def save(self, filename: str = None) -> None:
        """Save the plot.

        Args:
            filename(str): The path to the output png file.
        """
        # Create a plot.
        print('[SteinerPlot] Creating plot...')
        surface_width, surface_height = self._get_surface_size()
        surface = cairo.ImageSurface(
            cairo.FORMAT_ARGB32, surface_width, surface_height)
        context = cairo.Context(surface)

        # Set the background color (optional, to fill the canvas)
        context.set_source_rgb(1, 1, 1)  # White background
        context.paint()

        # Move origin to bottom-left corner and flip the y-axis (scale by -1)
        context.translate(0, surface_height)
        context.scale(1, -1)

        bdry_width, bdry_height = self._get_boundary_size()
        actual_width, actual_height = self._get_plot_boundary_size()

        # Set the scale and translate the origin.
        scale_x = surface_width / actual_width
        scale_y = surface_height / actual_height
        scale_factor = min(scale_x, scale_y)
        context.scale(scale_factor, scale_factor)

        # Shift to the center of the surface. Use the actual width and height.
        context.translate((surface_width / scale_factor - bdry_width) / 2,
                          (surface_height / scale_factor - bdry_height) / 2)

        # Adjust the linewidth based on the scale factor. Because the linewidth
        # will be scaled down by the same factor as the coordinates, we need to
        # scale it back up to maintain the same visual appearance.
        self._adjust_linewidth(scale_factor)

        # Draw the Steiner tree.
        # Boundary.
        if 'boundary' in self.data:
            self._draw_boundary(context)
        # Nodes.
        if 'nodes' in self.data:
            self._draw_nodes(context)
        # Edges.
        if 'edges' in self.data:
            self._draw_edges(context)

        # Make sure the png_name is not None.
        if filename is None:
            filename = SteinerPlot.DEFAULT_PNG_NAME
            print(f'[CairoPlot] No PNG file name specified. '
                  f'Saving to \'{filename}\'...')

        # Save the image to a png file
        surface.write_to_png(filename)
        print(f'[CairoPlot] Image saved to \'{filename}\'.')


def main() -> None:
    """Main function.

    Args:
        -i, --input (str): The path to the input file. (required)
        -o, --output (str): The path to the output file. (optional)
        -p, --png (str): The name of the output png file. (optional)
    """
    # Args parser.
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', action='store',
                        default=None, help='input file name')
    parser.add_argument('-o', '--output', action='store',
                        default=None, help='output file name')
    parser.add_argument('-p', '--png', action='store',
                        default=None,
                        help='output png file name')
    args = parser.parse_args()

    print(f'[Main] Input: {args.input}')
    print(f'[Main] Output: {args.output}')
    print(f'[Main] PNG: {args.png}')

    # Initialize the plotter.
    plotter = SteinerPlot()

    # Read the input file.
    if args.input is not None:
        plotter.read_input(args.input)
    else:  # No input file specified.
        print('[Main] No input file specified. Exiting...')
        return

    # Read the output file.
    if args.output is not None:
        plotter.read_output(args.output)

    # Save the Steiner tree to a png file.
    plotter.save(args.png)


if __name__ == '__main__':
    main()
