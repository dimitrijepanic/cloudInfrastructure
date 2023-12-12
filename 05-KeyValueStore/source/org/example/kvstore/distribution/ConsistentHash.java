package org.example.kvstore.distribution;

import org.jgroups.Address;
import org.jgroups.View;

import java.util.HashMap;
import java.util.Map;
import java.util.TreeSet;

public class ConsistentHash implements Strategy{

    private TreeSet<Integer> ring;
    private Map<Integer, Address> addresses;

    public ConsistentHash(View view){
    	this.ring = new TreeSet<>();
    	this.addresses = new HashMap<>();
    	for(Address address : view.getMembers()) {
    		this.addresses.put(address.hashCode(), address);
    		this.ring.add(address.hashCode());
    	}
    }

    @Override
    public Address lookup(Object key){
    	Integer addressHashCode = ring.ceiling((Integer)key);
    	if(addressHashCode == null) addressHashCode = ring.first();
    	return addresses.get(addressHashCode);
    }
}