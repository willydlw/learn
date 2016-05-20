import pygame
from pygame.sprite import Group 

from settings import Settings 
from ship import Ship
import game_functions as gf 

def run_game():
	# Initilize game and create a screen object
	pygame.init()
	ai_settings = Settings()

	# create a display window named screen
	screen = pygame.display.set_mode(
		(ai_settings.screen_width, ai_settings.screen_height))
	pygame.display.set_caption("Alien Invasion")

	# set background color 
	bg_color = (230, 230, 230)

	# Make a ship
	ship = Ship(ai_settings, screen)

	# Make a group to store the bullets in.
	bullets = Group()

	# Start the main loop for the game
	while True:

		# Watch for keyboard and mouse events
		gf.check_events(ai_settings, screen, ship, bullets)
		ship.update()
		gf.update_bullets(bullets)
		gf.update_screen(ai_settings, screen, ship, bullets)
	
run_game()