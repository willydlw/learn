import sys
import pygame

from bullet import Bullet 

def check_keydown_events(event, ai_settings, screen, ship, bullets):
	"""Respond to keypresses."""
	if event.key == pygame.K_RIGHT:
		ship.moving_right = True
	elif event.key == pygame.K_LEFT:
		ship.moving_left = True
	elif event.key == pygame.K_SPACE:
		# Create a new bullet and add it to the bullets group
		# when the spacebar is pressed
		if len(bullets) < ai_settings.bullets_allowed:
			new_bullet = Bullet(ai_settings, screen, ship)
			bullets.add(new_bullet) 

def check_keyup_events(event, ship):
	""" Respond to key releases."""
	if event.key == pygame.K_RIGHT:
		# right arrow key release, stop moving ship to the right
		ship.moving_right = False
	elif event.key == pygame.K_LEFT:
		# left arrow key release
		ship.moving_left = False


def check_events(ai_settings, screen, ship, bullets):
	"""Respond to keypresses and mouse events."""
	for event in pygame.event.get():
		if event.type == pygame.QUIT:
			sys.exit()
		elif event.type == pygame.KEYDOWN:
			# each keypress is registered as a keydown event
			check_keydown_events(event, ai_settings, screen, ship, bullets)		
		elif event.type == pygame.KEYUP:
			# key release is a KEYUP event
			check_keyup_events(event, ship)
			

def update_screen(ai_settings, screen, ship, bullets):
	"""Update the screen during each pass through the loop."""
	# Redraw the screen during each pass through the loop
	screen.fill(ai_settings.bg_color)
	ship.blitme()

	# Redraw all bullets behind ship and aliens
	# bullets.sprites() returns a list of all the sprites
	# in the group bullets
	for bullet in bullets.sprites():
		bullet.draw_bullet()

	# Make the most recently drawn screen visible
	pygame.display.flip()

def update_bullets(bullets):
	"""Update position of bullets and get rid of old bullet"""
	# update bullet positions
	bullets.update()

	# Get rid of bullets that have disappeared
	# Make a copy of the bullets list for iteration
	# 	because items are being removed from the original list
	for bullet in bullets.copy():
		if bullet.rect.bottom <= 0:
			bullets.remove(bullet)