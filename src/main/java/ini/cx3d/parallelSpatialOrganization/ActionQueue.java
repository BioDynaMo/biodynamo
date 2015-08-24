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

import java.io.Serializable;
import java.util.ArrayList;
import java.util.LinkedList;

public class ActionQueue<T> implements Serializable{
	int size = 0;
	ArrayList<LinkedList<CacheManager<T>>> list = new ArrayList<LinkedList<CacheManager<T>>>();
	public ActionQueue() {
	}
	
	private LinkedList<CacheManager<T>> getList(final int priority) {
		int i = 0; 
		while (i < list.size()) {
			int currentPriority = list.get(i).getFirst().getPriority(); 
			if (currentPriority == priority)
				return list.get(i);
			else if (currentPriority > priority) {
				break;
			}
			i++;
		}
		LinkedList<CacheManager<T>> ret = new LinkedList<CacheManager<T>>();
		// add dummy action at front:
		ret.add(new CacheManager<T>(null) {
			public int getPriority() {	return priority; }
			public void run() {}
			boolean executeTask() throws NodeLockedException {
				return false;
			}
			@Override
			public String descriptionString() {
				return "DummyAction";
			} });
		list.add(i,ret);
		return ret;
	}
	
	public void addAction(CacheManager<T> action) {
		getList(action.getPriority()).addLast(action);
		size++;
	}
	
	public CacheManager<T> nextAction() {
		for (LinkedList<CacheManager<T>> element : list) {
			if (element.size() > 1) {
				size--;
				return element.remove(1);
			}
		}
		return null;
	}
	
	public boolean isEmpty() {
		return this.size == 0;
	}
	
	public int getSize() {
		return size;
	}
}
 