package org.example.kvstore.distribution;

import java.util.List;

import org.jgroups.Address;
import org.jgroups.View;

public class RoundRobin implements Strategy {

	
	private List<Address> addresses;
	
	public RoundRobin(View view) {
		this.addresses = view.getMembers();
	}
	
	@Override
	public Address lookup(Object key) {
		return addresses.get(Math.abs((Integer) key  % addresses.size()));
	}
}
