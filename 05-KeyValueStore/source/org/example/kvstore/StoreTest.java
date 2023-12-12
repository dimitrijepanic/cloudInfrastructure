package org.example.kvstore;

import java.util.HashMap;
import java.util.Map;
import java.util.Random;

import org.junit.Test;


public class StoreTest {

//    @Test
//    public void baseOperations() {
//        StoreManager manager = new StoreManager();
//        Store<Integer, Integer> store = manager.newStore();
//
//        assert store.get(1) == null;
//
//        store.put(42, 1);
//        assert store.get(42).equals(1);
//
//        assert store.put(42, 2).equals(1);
//
//    }

//    @Test
//    public void multipleStores(){
//        int NCALLS = 1000;
//        Random rand = new Random(System.nanoTime());
//
//        StoreManager manager = new StoreManager();
//        Store<Integer, Integer> store1 = manager.newStore();
//        Store<Integer, Integer> store2 = manager.newStore();
//        Store<Integer, Integer> store3 = manager.newStore();
//
//        for (int i=0; i<NCALLS; i++) {
//            int k = rand.nextInt();
//            int v = rand.nextInt();
//            store1.put(k, v);
//            assert rand.nextBoolean() ? store2.get(k).equals(v) : store3.get(k).equals(v);
//        }
//        
//    }
//    
//    @Test
//    public void dataMigration(){
//        int NCALLS = 1000;
//        Random rand = new Random(System.nanoTime());
//
//        StoreManager manager = new StoreManager();
//        Store<Integer, Integer> store1 = manager.newStore();
//        Store<Integer, Integer> store2 = manager.newStore();
//        Store<Integer, Integer> store3 = manager.newStore();
//
//        Map<Integer, Integer> map = new HashMap<>();
//        for (int i=0; i<NCALLS; i++) {
//            int k = rand.nextInt();
//            int v = rand.nextInt();
//            store1.put(k, v);
//            map.put(k, v);
//            assert rand.nextBoolean() ? store2.get(k).equals(v) : store3.get(k).equals(v);
//        }
//        
//        Store<Integer, Integer> store4 = manager.newStore();
////        
//        for (Map.Entry<Integer, Integer> set :
//            map.entrySet()) {
//        	int k = set.getKey();
//        	int v = set.getValue();
//        	assert rand.nextBoolean() ? store4.get(k).equals(v) : store3.get(k).equals(v);
//        }
//    }
//    
    @Test
    public void strategyComparison(){
    	// consistent hash 
        int NCALLS = 1000;
        Random rand = new Random(System.nanoTime());
        StoreManager manager = new StoreManager();
        Store<Integer, Integer> store1ch = manager.newStore("ConsistentHashStore", 0);
        Store<Integer, Integer> store2ch = manager.newStore("ConsistentHashStore", 0);
        Store<Integer, Integer> store3ch = manager.newStore("ConsistentHashStore", 0);
        
        for (int i=0; i<NCALLS; i++) {
            int k = rand.nextInt();
            int v = rand.nextInt();
            store1ch.put(k, v);
            assert rand.nextBoolean() ? store2ch.get(k).equals(v) : store3ch.get(k).equals(v);
        }
        // for migration
        Store<Integer, Integer> store4ch = manager.newStore("ConsistentHashStore", 0);
        // round robin
        Store<Integer, Integer> store1r = manager.newStore("RoundRobinStore", 1);
        Store<Integer, Integer> store2r = manager.newStore("RoundRobinStore", 1);
        Store<Integer, Integer> store3r = manager.newStore("RoundRobinStore", 1);
        
        for (int i=0; i<NCALLS; i++) {
            int k = rand.nextInt();
            int v = rand.nextInt();
            store1r.put(k, v);
            assert rand.nextBoolean() ? store2r.get(k).equals(v) : store3r.get(k).equals(v);
        }
        // for migration
        Store<Integer, Integer> store4r = manager.newStore("RoundRobinStore", 1);
    }
// 

}