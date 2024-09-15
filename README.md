# Unreal Engine 5 Grenade Demo

## Video of this grenade class in action: 
### [YouTube Video](https://www.youtube.com/watch?v=fEDDmg47Mr0)

## Description:
This Unreal Engine 5 demo showcases an implementation of a grenade class.  This class includes the following features:
- An adjustable timed fuse before detonation.
- Ability to pull the pin on demand, after which the countdown starts, even if it's still in the player's hand.
- Basic bounce physics.
- Basic sounds for various grenade interactions and explosion.
- Ability to set how many child grenades to spawn on detonation, the children can in turns spawn more grandchildren, and so on...
- Travel trails can be turned on and off per blueprint, in my video it is disabled for the main grenade, but enabled for the child grenades.
- Knockback impulse with falloff, settings via Radial Force Component in blueprint.
- Radial damage with falloff, settings via blueprint.

## Reasons for various decisions:
- In a real project, the Grenade class would probably inherit from a parent Weapon class, but here it isn't for simplicity as no other weapons are being implemented in this project.
- Some of the private variables/functions may well need to be defined as protected or public depending on usage, and whether subclasses will require overriding, but I kept as much of it private as possible for now for safety/security until required.
