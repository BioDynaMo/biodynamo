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

package ini.cx3d.parallelSpatialOrganization;


import java.util.Iterator;
import java.util.Stack;

public class BinaryTreeElement<T> implements Iterable<SpaceNode<T>> {
	SpaceNode<T> content;
	BinaryTreeElement<T> smaller, bigger;
	int contentID;
	public BinaryTreeElement(SpaceNode<T> content) {
		this.content = content;
		if (content != null)
			contentID = getHash(content); 
//				content.getId();
		else 
			contentID = -1;
	}
	public String toString() {
		String ret = "";
		if (smaller != null)
			ret = smaller.toString();
		if (content != null) {
			if (ret != "")
				ret = ret + ", ";
			ret = ret + content;
		}
		if (bigger != null) {
			if (ret != "")
				ret = ret + ", ";
			ret = ret + bigger;
		}
		return ret;
	}
	private int getHash(SpaceNode<T> content) {
		return (int)(((long)content.getId()*(long)7481)%74317);
	}
	private boolean contains(int id, SpaceNode<T> node) {
		if (this.content.equals(node)) return true;
		else {
			if ((contentID >= id) && (smaller != null))
				return smaller.contains(id, node);
			else if ((contentID < id) && (bigger != null))
				return bigger.contains(id, node);
			else
				return false;
		}
	}
	public boolean contains(SpaceNode<T> content) {
		return contains(getHash(content), content);
	}
	
//	public boolean contains(SpaceNode content) {
//		if (this.content == content) return true;
//		else {
//			if ((contentID > content.getId()) && (smaller != null))
//				return smaller.contains(content);
//			else if ((contentID < content.getId()) && (bigger != null))
//				return bigger.contains(content);
//			else
//				return false;
//		}
//	}
	public void insert(SpaceNode<T> content) {
		if (content != null) {
			insert(new BinaryTreeElement<T>(content));
		}
//		if (this.content == content) return;
//		else {
//			if ((contentID > content.getId())) {
//				if ((smaller != null)) 
//					smaller.insert(content);
//				else
//					smaller = new BinaryTreeElement(content);
//			}
//			else if (contentID < content.getId()) {
//				if (bigger != null) {
//					bigger.insert(content);
//				}
//				else
//					bigger = new BinaryTreeElement(content);
//			}
//		}
	}
	private void insert(BinaryTreeElement<T> element) {
		if ((contentID == element.contentID) && (this.content.equals(element.content))) 
			return;
		else if ((contentID >= element.contentID)) {
			if ((smaller != null)) 
				smaller.insert(element);
			else
				smaller = element;
		}
		else if (contentID < element.contentID) {
			if (bigger != null) {
				bigger.insert(element);
			}
			else
				bigger = element;
		}
	}

	private void changeLink(BinaryTreeElement<T> oldEl, BinaryTreeElement<T> newEl) {
		if (smaller == oldEl)
			smaller = newEl;
		else if (bigger == oldEl)
			bigger = newEl;
	}
	private void remove(int id, SpaceNode<T> node, BinaryTreeElement<T> dad) {
		if ((this.contentID == id) && (this.content.equals(node))) {
			if ((smaller == null) && (bigger == null))
				dad.changeLink(this, null);
			else if ((smaller != null) && ((NewDelaunayTest.rand.nextDouble() < 0.5) || (bigger == null))) {
				dad.changeLink(this, smaller);
				if (bigger != null)
					smaller.insert(bigger);
			}
			else {
				dad.changeLink(this, bigger);
				if (smaller != null) 
					bigger.insert(smaller);
			}
		}
		else {
			if ((contentID >= id) && (smaller != null))
				smaller.remove(id, node, this);
			else if ((contentID < id) && (bigger != null))
				bigger.remove(id, node, this);
		}
	}
	public void remove(SpaceNode<T> content, BinaryTreeElement<T> dad) {
		remove(getHash(content), content ,dad);
	}

//	public void remove(SpaceNode content, BinaryTreeElement dad) {
//		if (this.content == content) {
//			if ((smaller == null) && (bigger == null))
//				dad.changeLink(this, null);
//			else if ((smaller != null) && ((Math.random() < 0.5) || (bigger == null))) {
//				dad.changeLink(this, smaller);
//				if (bigger != null)
//					smaller.insert(bigger);
//			}
//			else {
//				dad.changeLink(this, bigger);
//				if (smaller != null) 
//					bigger.insert(smaller);
//			}
//		}
//		else {
//			if ((contentID > content.getId()) && (smaller != null))
//				smaller.remove(content, this);
//			else if ((contentID < content.getId()) && (bigger != null))
//				bigger.remove(content,this);
//		}
//	}
	public Iterator<SpaceNode<T>> iterator() {
		final Stack<BinaryTreeElement<T>> elementStack = new Stack<BinaryTreeElement<T>>();
		BinaryTreeElement<T> dummy = this;
		while (dummy != null) {
			elementStack.push(dummy);
			dummy = dummy.smaller;
		}
		
		return new Iterator<SpaceNode<T>>() {
			public boolean hasNext() {
				return !elementStack.isEmpty();
			}
			public SpaceNode<T> next() {
				BinaryTreeElement<T> dummy = elementStack.pop();
				BinaryTreeElement<T> it = dummy.bigger;
				while (it != null) {
					elementStack.push(it);
					it = it.smaller;
				}
				return dummy.content;
			}

			public void remove() {
				throw new UnsupportedOperationException("Deletion from a tree is not implemented for this iterator!");
			}
		};
	}
	public static<T> BinaryTreeElement<T> generateTreeHead() {
		return new BinaryTreeElement<T>(null) {
			public boolean contains(SpaceNode<T> content) {
				if (bigger != null)	return bigger.contains(content); else return false;
			}
			public void insert(SpaceNode<T> content) {
				if (bigger != null) bigger.insert(content); else bigger = new BinaryTreeElement<T>(content);
			}
			public void remove(SpaceNode<T> content, BinaryTreeElement<T> dad) {
				if (bigger != null) bigger.remove(content, this);
			}
			public Iterator<SpaceNode<T>> iterator() {
				if (bigger != null) return bigger.iterator();
				else return new Iterator<SpaceNode<T>>() {
					public boolean hasNext() {	return false;	}
					public SpaceNode<T> next() { return null;  }
					public void remove() {}
				};
			}
		};
	}
}
