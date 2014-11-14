/*
 *
 **************************IMPORTANT*************************************
 *
 *		Run this test with 62KB of heap, use KVMutil to set the heap size.
 *
 */

import java.io.*;
import javax.microedition.io.*;
import com.sun.kjava.*;

class KvmHttpTest extends Spotlet {

	static Graphics g = Graphics.getGraphics();
	Button exitButton;

	public KvmHttpTest() {

		int count;
		int c;
		byte [] b = new byte[32];
		int total = 0;
		InputStream in = null;
		StreamConnection con = null;

		exitButton = new Button("Exit", 138, 146);
		g.clearScreen();
		g.drawString("Starting tests", 1, 1, g.PLAIN);
		exitButton.paint();

		g.drawString("Starting HTTP test", 1, 10, g.PLAIN);
		try {
			in = Connector.openInputStream("http://www.shore.net/~bpittore/mactest.html");

			while ((count = in.read(b, 0, 32)) > 0) {
				total += count;
			}
		} catch(IOException e) {
			System.out.println("HTTP read caught exception " + e);
			try {
				in.close();
			} catch (IOException x) {
				System.out.println("HTTP close exception " + x);
				System.exit(1);
			}
			System.exit(1);
		}

		g.drawString("HTTP test got " + total + " chars", 1, 20, g.PLAIN);
	//	System.out.println("got " + count + " chars");

		try {
			in.close();
		} catch (IOException e) {
			System.out.println("HTTP close exception " + e);
			System.exit(1);
		}

		g.drawString("Starting HTTPS test", 1, 30, g.PLAIN);
		try {
			in = Connector.openInputStream("https://east.sun.net");

			total = 0;
			while ((count = in.read(b, 0, 32)) > 0) {
				total += count;
			}
		} catch(IOException e) {
			System.out.println("HTTPS read caught exception " + e);
			try {
				in.close();
			} catch (IOException x) {
				System.out.println("HTTPS close exception " + x);
				System.exit(1);
			}
			System.exit(1);
		}

		g.drawString("HTTPS test got " + total + " chars", 1, 40, g.PLAIN);
//		System.out.println("got " + count + " chars");

		try {
			in.close();
		} catch (IOException e) {
			System.out.println("HTTPS close exception " + e);
			System.exit(1);
		}


		g.drawString("Starting HTTP POST test", 1, 50, g.PLAIN);

		try {
			con = (StreamConnection)Connector.open("http://www.shore.net/~bpittore/bin/samplecgi");

			OutputStream os = con.openOutputStream();

			os.close(); // POST w/no data
			in = con.openInputStream();
		} catch (IOException e) {
			System.out.println("HTTP POST exception " + e);
			try{
				in.close();
			} catch (IOException x) {
				System.out.println("HTTP POST close exception " + x);
				System.exit(1);
			}
			System.exit(1);
		}



		total = 0;
		try {
			while ((count = in.read(b, 0, 32)) > 0) {
				total += count;
			}
		} catch(IOException e) {
			System.out.println("HTTP POST caught exception " + e);
			try{
				in.close();
			} catch (IOException x) {
				System.out.println("HTTP POST close exception " + x);
				System.exit(1);
			}
			System.exit(1);
		}

		g.drawString("HTTP POST test got " + total + " chars", 1, 60, g.PLAIN);
//		System.out.println("got " + count + " chars");

		try{
			in.close();
		} catch (IOException e) {
			System.out.println("HTTP POST close exception " + e);
			System.exit(1);
		}
		g.drawString("Tests done", 1, 70, g.PLAIN);
	}

	public void penDown(int x, int y) {
		if (exitButton.pressed(x, y)) {
			System.exit(0);
		}
	}

    public static void main(String[] args) throws Throwable {

		(new KvmHttpTest()).register(NO_EVENT_OPTIONS);
	}

}
