# Part 1: Experience FPGA Acceleration

## Algorithm Overview  

 In this algorithm, you are monitoring a stream of incoming documents, and computing a score for each document based on the user’s interest, represented by a search array. An example system would be the use of a filtering mechanism, which monitors news feeds and sends relevant articles to a user.

This algorithm attempts to find the best matching documents for a specific search array. The search array is the filter that matches documents against the user’s interest. In this application, each document is reduced to a set of 32-bit words, where each word is a pair of 24-bit word ID and 8-bit frequency representing the occurrence of the word ID in the document. The search array consists of smaller set of word IDs and each word ID has a weight associated with it, which represents the significance of the word. The application computes a score for each document to determine its relevance to the given search array.

## Run the Application on the CPU

1. Navigate to the `cpu_src` directory and run the following command.

``` 
cd /home/centos/src/project_data/SDAccel-AWS-F1-Developer-Labs/modules/module_01/cpu_src
make run
```

2. The output is as follows.
 ```
 Total execution time of CPU                        |  4112.5895 ms
 Compute Hash & Output Flags processing time        |  3660.4433 ms
 Compute Score processing time                      |   452.1462 ms
--------------------------------------------------------------------
```

>**NOTE:** The performance number might vary depending on the CPU machine and workload activity at that time.

The above command computes the score for 100,000 documents (1.39 GB) in this application. The execution time for this application is 4.112 seconds. The throughput is as follows:

Throughput = Total data/Total time = 1.39 GB/4.112s = 338 MB/s

## Profiling the Application

To improve the performance of the application, you need to identify bottlenecks in this application where the application is spending majority of the time.

As we can see from the execution times in the previous step, the applications spends 89% of time in calculating the hash function and 11 % of the time in computing the document score.


## Evaluating What Code is a Good Fit for the FPGA

We have two files for evaluation:

1. compute_score_host.cpp : Computes document score based on the output flags created from hash function of the words. 

2. MurmurHash2.c : Called by `compute_score_host.cpp` to compute Hash function for a given word.


### Evaluating Hash Function 

1. Open `MurmurHash2.c` file with a file editor.

2. The `MurmurHash2` hash function code is as follows:

```
unsigned int MurmurHash2 ( const void * key, int len, unsigned int seed )
{
// 'm' and 'r' are mixing constants generated offline.
// They're not really 'magic', they just happen to work well.

const unsigned int m = 0x5bd1e995;
//	const int r = 24;

// Initialize the hash to a 'random' value

unsigned int h = seed ^ len;

// Mix 4 bytes at a time into the hash

const unsigned char * data = (const unsigned char *)key;

switch(len)
{
case 3: h ^= data[2] << 16;
case 2: h ^= data[1] << 8;
case 1: h ^= data[0];
  h *= m;
};

// Do a few final mixes of the hash to ensure the last few
// bytes are well-incorporated.

   h ^= h >> 13;
   h *= m;
   h ^= h >> 15;
 
 return h;
}   
```

* From the above code, you can see that the function reads a word from the memory and calculates hash output.

* The compute of hash for a single word ID consists of four XOR, 3 arithmetic shifts, and two multiplication operations.

* A shift of 1-bit in an arithmetic shift operation takes one clock cycle on the CPU.

* The three arithmetic operations shift a total of 44-bits (in the above code,`len=3` in above code) to compute the hash which requires 44 clock cycles just to shift the bits on CPU. Due to the custom hardware architecture possible on the FPGA, shifting by an arbitrary number of bits on the FPGA can complete the operation in one clock cycle.

* FPGA also has dedicated DSP units, which perform multiplication faster than the CPU. Even though the CPU runs at 8 times higher clock frequency than the FPGA, the arithmetic shift and multiplication operations can perform faster on FPGA because of its custom hardware architecture, enabling it to perform in fewer clock cycles compared to the CPU.

* Therefore this function is a good candidate for implemeting on FPGA.

3. Close the file.

### Evaluating Compute Output Flags from Hash Function


1. Open `compute_score_host.cpp` file in the file editor. 


2. The algorithm can be divided into two sections

   * Computing output flags created from the hash function of every word in all documents. 
  
   * Computing document score based on output flags created above.
   
   Let us evaluate if these code sections are a good fit for FPGA.

3.  The code at lines 32-58 which computes output flags is shown below.

```
// Compute output flags based on hash function output for the words in all documents

 for(unsigned int doc=0;doc<total_num_docs;doc++) 
    {
        profile_score[doc] = 0.0;
        unsigned int size = doc_sizes[doc];

        for (unsigned i = 0; i < size ; i++)
        { 
            unsigned curr_entry = input_doc_words[size_offset+i];
            unsigned word_id = curr_entry >> 8;
            unsigned hash_pu =  MurmurHash2( &word_id , 3,1);
            unsigned hash_lu =  MurmurHash2( &word_id , 3,5);
            bool doc_end = (word_id==docTag);
            unsigned hash1 = hash_pu&hash_bloom;
            bool inh1 = (!doc_end) && (bloom_filter[ hash1 >> 5 ] & ( 1 << (hash1 & 0x1f)));
            unsigned hash2 = (hash_pu+hash_lu)&hash_bloom;
            bool inh2 = (!doc_end) && (bloom_filter[ hash2 >> 5 ] & ( 1 << (hash2 & 0x1f)));
            
           
            if (inh1 && inh2) {
                inh_flags[size_offset+i]=1;
            }else {
                inh_flags[size_offset+i]=0;
            }
        }
      
        size_offset+=size;
    }

```


* From the above code, we see that we are computing two hash outputs for each word in all the documents and creating output flags         accordingly.

* We already saw that hash function is a good candidate for acceleration on FPGA.

* The computation of hash function of one word is independent of other words and can be done in parallel thereby improving the           execution time.

* The input words read from the DDR are accessed sequentially from DDR enablng FPGA to infer words from DDR in burst mode thereby         improving DDR read bandwidth.

 4. Close the file. 
 
 Looking at above code you can see that this code section is a a good candidate for FPGA as hash function can run faster on FPGA and     we can compute hash function for multiple words in parallel by reading multiple words from DDR in burst mode. 

### Evaluating Compute Document Score

The code for computing the document score is as follows:

```
for(unsigned int doc=0, n=0; doc<total_num_docs;doc++)
   {
      profile_score[doc] = 0.0;
      unsigned int size = doc_sizes[doc];

      for (unsigned i = 0; i < size ; i++,n++)
      {
          if(inh_flags[n])
        {
          unsigned curr_entry = input_doc_words[n];
          unsigned frequency = curr_entry & 0x00ff;
          unsigned word_id = curr_entry >> 8;
          profile_score[doc]+= profile_weights[word_id] * (unsigned long)frequency;
        }
      }
  }
```

* From the above code, you can see that the compute score requires one memory access to  `profile_weights`, one accumulation and one multiplication operation.

* The memory accesses are random in each loop iteration, because you do not know the word ID accessed  in each consecutive word of the document.

* The size of `profile_weights` array is 128 MB and is placed in FPGA DDR. Non-sequential accesses to DDR are big performance bottlenecks. Since accesses to the `profile_weights` array are random and since this function takes only about 11% of the total running time, we can run this function on the   CPU. If this function is implemented on the FPGA, the accesses to this array can slow down the   performance while computing the score, so you can keep this function on CPU.

Based on this analysis of the algorithm, you will not offload Compute Document Score code section and will only offload Compute Hash and Output Flags code section of `compute_score_fpga.cpp` on FPGA.

## Run the Application on the FPGA

When the Compute Hash & Output Flags code section is implemented in the FPGA, the FPGA returns a byte for each input document word received, indicating if the word is present in the search array. In this  implementation, we are processing 8 32-bit input document words in parallel computing hash and output flags for 8 words in every clock cycle. After offloading the compute hash and output flags code section on FPGA, you see the following results.

1. Run the following make command for running optimized application on FPGA

   `make run_fpga`


2  The output is as follows:

 ```
--------------------------------------------------------------------
 Executed FPGA accelerated version  |   552.5344 ms   ( FPGA 528.744 ms )
 Executed Software-Only version     |   3864.4070 ms
--------------------------------------------------------------------
 Verification: PASS
```

You can see that the execution time of the application has increased almost by a factor of 7 by offloading the compute hash and outout flags code section to the FPGA.  

## Conclusion

In the previous steps, we have seen total execution time on FPGA has improved 7 times compared to CPU. The optimization of 3 key components of an FPGA-accelerated application that helped achieve these results are :

* FPGA Kernel 
* Data Movement
* Host Application

In the next steps, we explain the techniques used to optimize data movement and the host application by [overlapping data transfers and compute](./data_movement.md) between host and FPGA. To further improve the performance of application, you overlap compute hash processing on FPGA with CPU document score to achieve an optimal execution time.
