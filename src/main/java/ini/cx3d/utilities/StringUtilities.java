/*
Copyright (C) 2009 Frédéric Zubler, Rodney J. Douglas,
Dennis Göhlsdorf, Toby Weston, Andreas Hauri, Roman Bauer,
Sabina Pfister & Adrian M. Whatley.

This file is part of CX3D.

CX3D is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CX3D is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with CX3D.  If not, see <http://www.gnu.org/licenses/>.
*/

package ini.cx3d.utilities;

import static ini.cx3d.utilities.SystemUtilities.*;
;
/**
 * Some static methods to handle String manipulation, for finding certain character etc.
 * Is mainly used by parsers.
 * @author fredericzubler
 *
 */
public abstract class StringUtilities {


	//--------------------------------------------------
	// some small utilities
	//---------------------------------------------------
	
	/**
	 * Returns a String containing the value of the double with (at most) 
	 * the specified number of digits (rounded if needed).
	 * Ex : doubleToString(214.1234567,4) returns "214.1235",
	 * doubleToString(3.5,5) returns "3.5".
	 * @param val the double you want to express
	 * @param digits the max number of digits that you want
	 * @return
	 */
	public static String doubleToString(double val, int digits){
		double d = Math.pow(10, digits);
		val *= d;							// to avoid decimals
		val = Math.round(val);
		val/= d;
		return Double.toString(val);
	}

	/** Return a string similar, but without the blanks and the returns.*/
	public static String removeBlanks(String string){
		String s = string.replace(" ", "");
		s.replace("\n", "");
		return s;
	}
	
	//--------------------------------------------------
	// Few things need for parser
	//---------------------------------------------------
	
	
	
	/** Returns the value of the position index of the next char ' in the string.
	 * @param instructionString
	 * @param startingIndex index position of the first '
	 * @return
	 */
	public static int findIndexfOfNextApostrophy(String instructionString, int startingIndex){
		int lengthOfTheString = instructionString.length();
		for (int index = startingIndex+1  ; index < lengthOfTheString; index++) {
			char c = instructionString.charAt(index);
			if(c == '\''){ 				
				return index;
			}
		}
		return startingIndex + 1;
	}


	/** Returns the value of the position index of the next char c in the string,
	 * starting at value index.
	 * Ex : findIndexfOfNext("sadda",1,'a') returns 1 ; findIndexfOfNext("sadda",2,'a') returns 4 
	 * 
	 * @param string
	 * @param startingIndex index position of the first '
	 * @return
	 */
	public static int findIndexfOfNext(String string, int startingIndex, char c){
		int lengthOfTheString = string.length();
		for (int index = startingIndex  ; index < lengthOfTheString; index++) {
			char cc = string.charAt(index);
			if(cc == c){ 				
				return index;
			}
		}
		return -1;
	}
	
	/** Returns the value of the position index of the next char c in the string,
	 * starting at value index.
	 * Ex : findIndexfOfNext("sadda",1,'a') returns 1 ; findIndexfOfNext("sadda",2,'a') returns 4 
	 * 
	 * @param string
	 * @param startingIndex index position of the first '
	 * @return
	 */
	public static int findIndexfOfNext(String string, int startingIndex, char ... c){
		int lengthOfTheString = string.length();
		for (int index = startingIndex  ; index < lengthOfTheString; index++) {
			char cc = string.charAt(index);
			for (int i = 0; i < c.length; i++) {
				if(cc == c[i]){ 				
					return index;
				}
			}
			
		}
		return -1;
	}
	

	/**
	 * Returns index of first letter that is not a digit
	 * @param instructionString
	 * @param startingIndex
	 * @return
	 */
	public static int findIndexfOfEndOfNumeral(String instructionString, int startingIndex){
		int lengthOfTheString = instructionString.length();
		boolean alreadyGotAPoint = false; // like in 45.345
		for (int index = startingIndex  ; index < lengthOfTheString; index++) {
			char c = instructionString.charAt(index);
			if(Character.isDigit(c))
				continue;
			if(c == '.' && alreadyGotAPoint == false){
				alreadyGotAPoint = true;
				continue;
			}
			// Here we encounter a non numerical char :
			String s = instructionString.substring(startingIndex, index);
			System.out.println("index = "+index+", s = "+s);
			return index;
		}
		// Here we have parsed the whole String, and all char are digits
		return lengthOfTheString;
	}

	/**
	 * Returns the substring (isolated by ') starting at position "index". 
	 * Example: ("look'up'and'down'please", 7) returns "and". 
	 * @param instructionString the String in which we look
	 * @param index the position of the first '
	 * @return Substring
	 */
	public static String ExtractNextName(String instructionString, int startingIndex){
		int lengthOfTheString = instructionString.length();
		for (int index = startingIndex+1  ; index < lengthOfTheString; index++) {
			char c = instructionString.charAt(index);
			if(c == '\''){ 				// there's a closing bracket :
				return instructionString.substring(startingIndex+1, index);
			}
		}
		return "";
	}


//	/**
//	 * Returns a substring representing a double. Ex : ("as34.39sdf45f",2) returns 34.39
//	 * @param instructionString
//	 * @param startingIndex index position of the first digit
//	 * @return
//	 */
//	public static Double ExtractNextNumber(String instructionString, int startingIndex){
//		int lengthOfTheString = instructionString.length();
//		boolean alreadyGotAPoint = false; // like in 45.345
//		for (int index = startingIndex+1  ; index < lengthOfTheString; index++) {
//			char c = instructionString.charAt(index);
//			if(Character.isDigit(c))
//				continue;
//			if(c == '.' && alreadyGotAPoint == false){
//				alreadyGotAPoint = true;
//				continue;
//			}
//			
//			if()
//			// it is the end of the number :
//			String s = instructionString.substring(startingIndex, index);
//			return Double.parseDouble(s);
//
//		}
//		return Double.NaN;
//	}
	
	/**
	 * Returns a substring representing a double. Ex : ("as34.39sdf45f",2) returns 34.39
	 * @param instructionString
	 * @param startingIndex index position of the first digit
	 * @return
	 */
	public static Double ExtractNextNumber(String instructionString, int startingIndex){
		int lengthOfTheString = instructionString.length();
		boolean alreadyGotAPoint = false; // like in 45.345
		for (int index = startingIndex+1  ; index < lengthOfTheString; index++) {
			char c = instructionString.charAt(index);
			if(Character.isDigit(c) && index < lengthOfTheString)
				continue;
			if(c == '.' && alreadyGotAPoint == false && index < lengthOfTheString){
				alreadyGotAPoint = true;
				continue;
			}
			
			
			// it is the end of the number :
			String s = instructionString.substring(startingIndex, index);
			return Double.parseDouble(s);
			
		}
		return Double.NaN;
	}
	
	// --------------------------------------------------------------------
	// Find closing (), [] or {}
	// --------------------------------------------------------------------
	
	/**
	 * Finds the closing curly bracket '}' closing the opening '{' at position "index"
	 * @param instructionString the String in which we look
	 * @param index the position of the {
	 * @return index of the closing }
	 */
	public static int findCorrespondingCurlyBracket(String instructionString, int index){
		int lengthOfTheString = instructionString.length();
		int level = 0; // sub brackets that can be opened
		for (index++ ; index < lengthOfTheString; index++) {
			char c = instructionString.charAt(index);
			if(c == '}'){ 				// there's a closing bracket :
				if(level == 0){ 		// if it is at the same level, we have what we want
					return index;
				}else{					// otherwise we update the level
					level +=1 ;
				}
			}else if(c == '{'){			// if it is another opening one, we close also change the level
				level -= 1;
			}
		}
		return -1;
	}
	
	
	/**
	 * Finds the closing square bracket ] closing the opening one [ at position index
	 * @param instructionString the String in which we look
	 * @param index the position of the [
	 * @return index of the closing ]
	 */
	public static int findCorrespondingSquareBracket(String instructionString, int index){
		int lengthOfTheString = instructionString.length();
		int level = 0; // sub brackets that can be opened
		for (index++ ; index < lengthOfTheString; index++) {
			char c = instructionString.charAt(index);
			if(c == ']'){ 				// there's a closing bracket :
				if(level == 0){ 		// if it is at the same level, we have what we want
					return index;
				}else{					// otherwise we update the level
					level +=1 ;
				}
			}else if(c == '['){			// if it is another opening one, we close also change the level
				level -= 1;
			}
		}
		return -1;
	}
	
	/**
	 * Finds the closing parenthesis ) closing the opening one ( at position index
	 * @param instructionString the String in which we look
	 * @param index the position of the (
	 * @return index of the closing )
	 */
	public static int findCorrespondingParenthesis(String instructionString, int index){
		int lengthOfTheString = instructionString.length();
		int level = 0; // sub brackets that can be opened
		for (index++ ; index < lengthOfTheString; index++) {
			char c = instructionString.charAt(index);
			if(c == ')'){ 				// there's a closing bracket :
				if(level == 0){ 		// if it is at the same level, we have what we want
					return index;
				}else{					// otherwise we update the level
					level +=1 ;
				}
			}else if(c == '('){			// if it is another opening one, we close also change the level
				level -= 1;
			}
		}
		return -1;
	}
	
	
	
	public static void main(String[] args) {
		String a = " q weight e\n r     d fgfhfjhfjfjfklkdjdlkjd dsdsdsdssds\ndejkhdkejdhek";
		String b = null;
		tic();
		for (int i = 0; i < 1000; i++) {
//			b = a.replaceAll("\\s*", "");
			b = removeBlanks(a);
		}
		tac();
		System.out.println(b);
	}

}
