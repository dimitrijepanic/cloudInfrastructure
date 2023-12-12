package org.example.kvstore;

import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.example.kvstore.cmd.Command;
import org.example.kvstore.cmd.CommandFactory;
import org.example.kvstore.cmd.Get;
import org.example.kvstore.cmd.Put;
import org.example.kvstore.cmd.Reply;
import org.example.kvstore.distribution.ConsistentHash;
import org.example.kvstore.distribution.RoundRobin;
import org.example.kvstore.distribution.Strategy;
import org.jgroups.Address;
import org.jgroups.JChannel;
import org.jgroups.Message;
import org.jgroups.ReceiverAdapter;
import org.jgroups.View;

public class StoreImpl<K,V> extends ReceiverAdapter implements Store<K,V> {

    private String name;
    private Strategy strategy;
    private Map<K,V> data;
    private CommandFactory<K,V> factory;
    private JChannel channel;
    private ExecutorService workers;
    private CompletableFuture pending;
    private int hashType;
    
    private class CmdHandler implements Callable<Void>{

    	private Address address;
    	private Command cmd;
    	
    	public CmdHandler(Address address, Command cmd) {
    		this.address = address;
    		this.cmd = cmd;
    	}
    	
		@Override
		public Void call() throws Exception {
			if(cmd.getClass().equals(Reply.class)) {
				synchronized(pending) {
					StoreImpl.this.pending.complete(cmd.getValue());
				}
				return null;
			}
			
			V data = null;
			K key =(K) cmd.getKey();
			
			if(cmd.getClass().equals(Get.class)) {
				data = StoreImpl.this.data.get(key);
			} 
			
			if(cmd.getClass().equals(Put.class)) {
				data = StoreImpl.this.data.put(key,(V)cmd.getValue());
			}
			
			StoreImpl.this.send(address, StoreImpl.this.factory.newReplyCmd(key, data));
			return null;
		}
    	
    }

    public StoreImpl(String name, int hashType) {
        this.name = name;
        this.hashType = hashType;
    }

    
    public void init() throws Exception{
    	this.data = new ConcurrentHashMap<>();
    	this.workers = Executors.newCachedThreadPool();
    	this.factory = new CommandFactory<>();
    	channel=new JChannel();
    	channel.setReceiver(this);
    	channel.connect(this.name);
    }

    private V execute(Address dst,Command cmd) {
    	this.pending = new CompletableFuture<>();
    	send(dst, cmd);
    	
    	try {
			return  (V) this.pending.get();
		} catch (Exception e) {
			e.printStackTrace();
		} 
    	
    	return null;
    }
    
    @Override
    public V get(K k) {
    	Address dst = strategy.lookup(k);
    	
    	if(dst.equals(this.channel.getAddress())){
    		return this.data.get(k);
    	}
    	return execute(dst, factory.newGetCmd(k));
    }

    @Override
    public V put(K k, V v) {
    	Address dst = strategy.lookup(k);
    	
    	if(dst.equals(this.channel.getAddress())){
    		return this.data.put(k, v);
    	}
    	return execute(dst, factory.newPutCmd(k, v));
    }

    private void checkDataMigration() {
    	Map<K, V> newMap = new ConcurrentHashMap<>();
    	Set<Entry<K,V>> entrySet = this.data.entrySet();
    	Address dst = null;
    	for(Entry<K,V> entryset : entrySet) {
    		Address address = this.strategy.lookup(entryset.getKey());
    		if(address.equals(this.channel.getAddress())) continue;
    		if(dst == null) dst = address;
    		this.data.remove(entryset.getKey());
    		newMap.put(entryset.getKey(), entryset.getValue());
    	}
    	
    	if(newMap.isEmpty()) return;
    	try {
			this.channel.send(new Message(dst, null, newMap));
		} catch (Exception e) {
			e.printStackTrace();
		}
    }
     
    @Override
    public void viewAccepted(View new_view) {
    	switch (this.hashType) {
    		case 0: this.strategy = new ConsistentHash(new_view); break;
    		case 1: this.strategy = new RoundRobin(new_view); break;
    	}
        checkDataMigration();
    }
    
    public void send(Address dst, Command cmd) {
    	try {
			channel.send(new Message(dst, null, cmd));
		} catch (Exception e) {
			e.printStackTrace();
		}
    }
    
    private void addMigrationData(ConcurrentHashMap map) {
    	this.data.putAll(map);
    	String method = "";
		switch (hashType) {
		case 0: method = "Consistent Hashing"; break;
		case 1: method = "Round Robin"; break;
		}
		System.out.println("Migrated: "+ method + " " + map.size());
    }
    
    @Override
    public void receive(Message msg) {
    	if(msg.getObject().getClass().equals(ConcurrentHashMap.class)) {
    		addMigrationData((ConcurrentHashMap)msg.getObject());
    	} else {
	    	CmdHandler cmdHandler = new CmdHandler(msg.getSrc(), (Command)msg.getObject());
	    	this.workers.submit(cmdHandler);
    	}
    }
    
    @Override
    public String toString(){
        return "Store#"+name+"{"+data.toString()+"}";
    }
    
    public void close() {
   	 	channel.close();
    }
}