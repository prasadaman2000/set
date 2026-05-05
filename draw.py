import pygame
import time

RED = (255, 0, 0)
GREEN = (0, 255, 0)
BLUE = (0, 0, 255)
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
GRAY = (180, 180, 180)


PADDING = 10
SHAPE_HEIGHT = 40

color_list = [RED, GREEN, BLUE]


def get_striped_surface(color):
    stripes = pygame.Surface((SHAPE_HEIGHT, SHAPE_HEIGHT), pygame.SRCALPHA)
    stripes.fill(WHITE)  # Background color
    for x in range(0, SHAPE_HEIGHT, 2):
        if x % 4 == 0:
            # Draw a thick line or small rect for each stripe
            pygame.draw.rect(stripes, color, (0, x, SHAPE_HEIGHT, 1))
    return stripes


def draw_polygon(surface, points, color, fill):
    if fill == "empty":
        pygame.draw.polygon(surface, color, points, width=2)
    elif fill == "solid":
        pygame.draw.polygon(surface, color, points)
    elif fill == "striped":
        min_x = min(p[0] for p in points)
        min_y = min(p[1] for p in points)
        rel_points = [(p[0] - min_x, p[1] - min_y) for p in points]
        stripes = get_striped_surface(color)
        mask_surface = pygame.Surface((SHAPE_HEIGHT, SHAPE_HEIGHT), pygame.SRCALPHA)
        pygame.draw.polygon(mask_surface, (255, 255, 255, 255), rel_points)
        stripes.blit(mask_surface, (0, 0), special_flags=pygame.BLEND_RGBA_MIN)
        surface.blit(stripes, (points[0][0], points[0][1]))
        pygame.draw.polygon(surface, color, points, width=2)


def draw_triangle(surface, x, y, color, fill="solid"):
    points = ((x, y), (x, y + SHAPE_HEIGHT), (x + SHAPE_HEIGHT, y + SHAPE_HEIGHT))
    draw_polygon(surface, points, color, fill)


def draw_square(surface, x, y, color, fill="solid"):
    points = (
        (x, y),
        (x, y + SHAPE_HEIGHT),
        (x + SHAPE_HEIGHT, y + SHAPE_HEIGHT),
        (x + SHAPE_HEIGHT, y),
    )
    draw_polygon(surface, points, color, fill)


def draw_circle(surface, x, y, color, fill="solid"):
    radius = SHAPE_HEIGHT // 2
    if fill == "solid":
        pygame.draw.circle(surface, color, (x + radius, y + radius), radius, width=0)
    elif fill == "empty":
        pygame.draw.circle(surface, color, (x + radius, y + radius), radius, width=2)
    elif fill == "striped":
        stripes = get_striped_surface(color)
        mask_surface = pygame.Surface((SHAPE_HEIGHT, SHAPE_HEIGHT), pygame.SRCALPHA)
        pygame.draw.circle(mask_surface, (255, 255, 255, 255), (radius, radius), radius)
        stripes.blit(mask_surface, (0, 0), special_flags=pygame.BLEND_RGBA_MIN)
        surface.blit(stripes, (x, y))
        pygame.draw.circle(surface, color, (x + radius, y + radius), radius, width=2)


shape_funcs = [draw_square, draw_circle, draw_triangle]
colors = [RED, GREEN, BLUE]
fills = ["empty", "solid", "striped"]


# 160 x 60 white rectangle
def draw_card(surface, idx, color, shape, fill, count):
    tl_x, tl_y = (
        PADDING + (idx % 3) * 160 + PADDING * (idx % 3),
        PADDING + (idx // 3) * 70 + PADDING * (idx // 3),
    )

    pygame.draw.rect(surface, WHITE, (tl_x, tl_y, 160, 60))
    if count == 0:
        pass
    elif count == 1:
        shape_funcs[shape](surface, tl_x + 60, tl_y + 10, colors[color], fills[fill])
    elif count == 2:
        shape_funcs[shape](
            surface, tl_x + (80 / 3), tl_y + 10, colors[color], fills[fill]
        )
        shape_funcs[shape](
            surface, tl_x + 40 + 2 * (80 / 3), tl_y + 10, colors[color], fills[fill]
        )
    elif count == 3:
        shape_funcs[shape](surface, tl_x + 10, tl_y + 10, colors[color], fills[fill])
        shape_funcs[shape](surface, tl_x + 60, tl_y + 10, colors[color], fills[fill])
        shape_funcs[shape](surface, tl_x + 110, tl_y + 10, colors[color], fills[fill])

    my_font = pygame.font.SysFont("Arial", 10)
    text_surface = my_font.render(f"{idx}", True, (0, 0, 0))
    surface.blit(text_surface, (tl_x + 2, tl_y + 2))


if __name__ == "__main__":
    # Initialize Pygame
    pygame.init()

    # Set up the game window
    screen = pygame.display.set_mode((520, 800))
    pygame.display.set_caption("Set")

    running = True
    while running:
        screen.fill((0, 0, 0))
        with open("display.txt", "r") as f:
            for idx, line in enumerate(f.readlines()):
                line = line.strip()
                color, shape, fill, count = line.split(" ")
                draw_card(
                    screen,
                    idx,
                    int(color),
                    int(shape),
                    int(fill),
                    int(count),
                )

        pygame.display.flip()
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        time.sleep(1)

    pygame.quit()
