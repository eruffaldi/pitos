/*
 *  Copyright (c) 1999 Sun Microsystems, Inc., 901 San Antonio Road,
 *  Palo Alto, CA 94303, U.S.A.  All Rights Reserved.
 * 
 *  Sun Microsystems, Inc. has intellectual property rights relating
 *  to the technology embodied in this software.  In particular, and
 *  without limitation, these intellectual property rights may include
 *  one or more U.S. patents, foreign patents, or pending
 *  applications.  Sun, Sun Microsystems, the Sun logo, Java, KJava,
 *  and all Sun-based and Java-based marks are trademarks or
 *  registered trademarks of Sun Microsystems, Inc.  in the United
 *  States and other countries.
 *
 *  This software is distributed under licenses restricting its use,
 *  copying, distribution, and decompilation.  No part of this
 *  software may be reproduced in any form by any means without prior
 *  written authorization of Sun and its licensors, if any.
 *
 *  FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 *  Subject to Standard License Terms and Conditions
 */

 

import com.sun.kjava.*;


// Simple game application: steer a rocket
// and avoid hitting the moving stars


// Two different star creation strategies have
// been implemented.  One approach uses pre-created
// star objects to minimize garbage collection, and
// the other instantiates new star objects whenever
// necessary.  As obvious, the latter approach generates
// a lot of garbage, making this a good stress test 
// program for the garbage collector.


public class StarCruiser extends Spotlet {

/*==============================================================================
 * Constants
 *============================================================================*/


	// Maximum number of stars simultaneously on the screen
	static final int MAXSTARS = 30;


	// Maximum number of ships available in the game
	static final int NUMBEROFSHIPS = 6;


	// Interval for printing game status (3 seconds)
	static final int STATUSINTERVAL = 3000;


	// Interval for speeding up the game (10 seconds)
	static final int SPEEDUPINTERVAL = 10000;


	// Minimum interval for creating new starts (0.2 seconds)
	static final int MINIMUMSTARINTERVAL = 200;


	// Width of the ship bitmap in pixels
	static final int BITMAPWIDTH = 8;


/*==============================================================================
 * Static variables
 *============================================================================*/

	// random number generator
	static java.util.Random random = new java.util.Random(); 

	// Handle on the singleton Graphics object
	static Graphics g = Graphics.getGraphics();


	// Starship bitmap
	static Bitmap shipBitmap = new Bitmap((short)1, new byte[] {
		(byte)0x70,
		(byte)0x8C,
		(byte)0x82,
		(byte)0x81,
		(byte)0x81,
		(byte)0x82,
		(byte)0x8C,
		(byte)0x70 });


	// Starship game (spotlet) instance
	static StarCruiser StarCruiserGame;

	static ScrollTextBox stb;


	// Play status
	static boolean play;


	// The GUI buttons
	static Button exitButton;
	static Button playButton;


/*==============================================================================
 * Instance variables
 *============================================================================*/


	// A array of star objects
	MovingStar[] stars;


	// The number of stars currently on display
	int numberOfStars;


	// Height of the ship
	int shipHeight;


	// The number of times the ship has hit a star
	int collisions;


    // Game start time
	long gameStartTime;


    // Scrolling delay
	int scrollDelay;


    // Star creation interval
	int starInterval;


/*==============================================================================
 * Static methods
 *============================================================================*/


	/**
	 * The main method.
	 */

	public static void main(String[] args) {

		// Instantiate game
		StarCruiserGame = new StarCruiser();

		String helpText = "Welcome to the Star Cruiser game!\nThe object of the Star Cruiser game is to avoid the oncoming asteroids with your Starcruiser ship and as you avoid them you get points (the score is shown in the middle of the screen).  You have six lives (depicted in the lower left hand part of the screen) so use them sparingly.  As you avoid more asteroids the asteroids come at you faster and faster.  To play the game press the scroll button in the middle to move up and down (hold them down for faster movement), thus avoiding the oncoming asteroids.\n\nHave fun playing and may the asteroids be with you and not hitting you!";

		stb = new ScrollTextBox(helpText, 5, 5, 150, 130);

		g.clearScreen();
		stb.paint();
		playButton.paint();

		// Register event handlers
		StarCruiserGame.register(NO_EVENT_OPTIONS);

		while (!play) 
			Thread.yield();

		// Run the non-event-handling part of the app
		StarCruiserGame.letTheGameRoll();
	}



/*==============================================================================
 * Constructors
 *============================================================================*/


	/**
	 * Constructor.
	 */

	public StarCruiser() {
		// Initialize the array of stars
		stars = new MovingStar[MAXSTARS];

		/* Alternative strategy: pre-create all stars 
		   (minimizes garbage collection)
		for (int i = 0; i < MAXSTARS; i++) stars[i] = new MovingStar();
		*/

		// Number of stars is initially 0
		numberOfStars = 0;


		// Initialize the number of collisions
		collisions = 0;


		// Initialize ship location
		shipHeight = 75;


		// Initialize scrolling delay (initially 0.2 seconds)
		scrollDelay = 200;

		// Initialize the maximum star creation interval 
		// (initially 1 second)
		starInterval = 1000;


		// Create the necessary GUI buttons
		exitButton = new Button("Exit", 138, 146);
		playButton = new Button("Done", 60, 145);
    }



/*==============================================================================
 * Instance methods
 *============================================================================*/

	/**
	 * Game main routine.  Generates new star 
	 * objects randomly, scrolls them left, and 
	 * checks for collisions with the ship. 
	 * 
	 * Note that the operations in this function 
	 * could have been implemented as separate threads,
	 * but let's use a more traditional solution
	 * this time.
	 */

	public void letTheGameRoll() {
		// Get clock time at game initialization
		long scrollTime = gameStartTime;
		long newStarTime = scrollTime;
		long speedupTime = scrollTime;
		long statusTime = scrollTime;

		// Calculate a random value when to create
		// a new star

		int randomInterval = (random.nextInt()>>>1) % starInterval;

		while (true) {
			long timeNow = System.currentTimeMillis();
			// Generate a new star at random intervals
			if (timeNow > newStarTime + randomInterval) {
				createNewStar();
				randomInterval = (random.nextInt()>>>1) % starInterval;
				newStarTime = timeNow;
			}


			// If more than 'scrollDelay' time has passed
			// scroll all the stars left and check for
			// collisions.
			if (timeNow > scrollTime + scrollDelay) {
				scrollStarsLeft();
				scrollTime = timeNow;

				// Paint the ship
				paintShip();

				// Make game a little bit faster
				// as the game progresses.
				if (timeNow > speedupTime + SPEEDUPINTERVAL) {
					scrollDelay -= 10;
					if (scrollDelay <= 10) 
						scrollDelay = 10;

					starInterval -= 50;
					if (starInterval <= MINIMUMSTARINTERVAL) 
						starInterval = MINIMUMSTARINTERVAL;


					// After two minutes, make stars move faster
					if (timeNow > gameStartTime + 12000)
						MovingStar.boostSpeed();
					speedupTime = timeNow;
				}

				// Update score periodically.
				if (timeNow > statusTime + STATUSINTERVAL) {
					paintScore();
					statusTime = timeNow;
				}
			}
		}
	}


	/**
	 * Scroll all the stars on the display left
	 * and check for collisions.
	 */

	private void scrollStarsLeft() {
		for (int i = 0; i < numberOfStars; i++) {
			stars[i].scrollLeft();

			// Check for collision with the ship
			if (stars[i].hitShip(shipHeight)) {
				collisions++;
				paintCollisionStatus();
				if (collisions >= NUMBEROFSHIPS) 
					exitGame();
				stars[i].undraw();
				removeStar(i);
			}

			// If the star went past the visible screen
			// area, make it inactive.  Note: we assume
			// that all the stars move at the same speed,
			// i.e., stars at the beginning of the array
			// are always removed before stars located
			// later in the array.
			else if (stars[i].wentBy()) {
				stars[i].undraw();
				removeStar(i);
			}
		}
	}


	/**
	 * Create a new star object
	 */

	private void createNewStar() {
		if (numberOfStars < MAXSTARS) {
			stars[numberOfStars++] = new MovingStar();
		}



/* Alternative strategy with pre-created stars (minimizes garbage collection)
		if (numberOfStars < MAXSTARS) {
            stars[numberOfStars++].initialize();
			}
*/
	}


	/**
	 * Remove the i'th star from list of stars
	 */

	private void removeStar(int i) {
		// Let's create garbage to stress-test the collector
		stars[i] = stars[numberOfStars-1];
		stars[numberOfStars-1] = null;
		if (stars[i] != null)
			stars[i].scrollLeft();

/* Alternative strategy with pre-created stars
   (minimizes garbage collection)
		if (stars[i] != stars[numberOfStars-1]) {
			// Remove the star by swapping it with another 
			// one, then decrease star count
			MovingStar temp = stars[i];
			stars[i] = stars[numberOfStars-1];
		    stars[i].scrollLeft();
			stars[numberOfStars-1] = temp;
		}
*/
		numberOfStars--;
	}



	/**
	 * Draw the ship in the current location.
	 */

	private void paintShip() {
		g.drawBitmap(1, shipHeight, shipBitmap);
	}


	/**
	 * Undraw the ship in the current location.
	 */

	private void unpaintShip() {
		g.drawRectangle(1, shipHeight, BITMAPWIDTH, BITMAPWIDTH, g.ERASE, 0);
	}


	/**
	 * Refresh the collision counter shown on the screen.
	 */

	private void paintCollisionStatus() {
		// Draw the number of collisions
		g.drawString("Collisions: " + collisions + "/" + NUMBEROFSHIPS, 1, 149);
		paintScore();
	}


	/**
	 * Refresh the score shown on the screen.
	 */

	private void paintScore() {
		// Draw the score and the number of collisions
		long timeNow = System.currentTimeMillis();
		g.drawString("Score: " + (int)(timeNow-gameStartTime)/10, 70, 149);
	}


	/**
	 * Get current ship location
	 */

	public int getShipHeight() {
		return shipHeight;
	}


	/**
	 * Move ship lower
	 */

	public void moveShipLower() {
		unpaintShip();
		shipHeight += 2;
		if (shipHeight >= 138) shipHeight = 138; 
		paintShip();
	}


	/**
	 * Move ship higher
	 */

	public void moveShipHigher() {
		unpaintShip();
		shipHeight -= 2;
		if (shipHeight <= 0) shipHeight = 0;
		paintShip();
	}


	/**
	 * Exit game.
	 */

	public void exitGame() {
		g.drawString("GAME OVER", 55, 70);

		// Wait 5 seconds, then exit
		try {	
			Thread.sleep(5000);
		} catch (InterruptedException e) {}
		System.exit(0);
	}



/*==============================================================================
 * Event handlers
 *============================================================================*/


	/**
	 * Handle a pen down event.
	 */

	public void penDown(int x, int y) {
        // handle prelude event (the prelude form)
        if ((! play) && playButton.pressed(x,y)) {
		    // Clear the display
		    g.clearScreen();

			// Draw the exit button
			exitButton.paint();

			// Print initial game status
			gameStartTime = System.currentTimeMillis();
			paintCollisionStatus();
			paintScore();

            // change mode
            play = true;			
		}
		if ((!play) && stb.contains(x,y)) {
			stb.handlePenDown(x,y);
		}

		if (play && exitButton.pressed(x,y)) {
				System.exit(0);
		}
	}

	public void penMove(int x, int y) {
		if((!play) && stb.contains(x,y)){
			stb.handlePenMove(x, y);
		}
	}


	/**
	 * Handle a key down event.
	 */

	public void keyDown(int keyCode) {
		if (!play) {
			stb.handleKeyDown(keyCode);
			return;
		}

		switch (keyCode) {
			case PAGEDOWN: 
				moveShipLower();
				break;

			case PAGEUP: 
				moveShipHigher();
				break;
		}
	}
}








