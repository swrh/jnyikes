package org.jnyikes;

public class JNyIkes {
	native int sendString(String);

	public static void load() {
		System.loadLibrary("jnyikes");
	}
}
