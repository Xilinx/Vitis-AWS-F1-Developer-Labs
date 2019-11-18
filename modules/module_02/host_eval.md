# Part 1: Experience FPGA Acceleration

## Algorithm Overview  

 In this algorithm, you are monitoring a stream of incoming documents, and computing a score for each document based on the user’s interest, represented by a search array. An example system would be the use of a filtering mechanism, which monitors news feeds and sends relevant articles to a user.

This algorithm attempts to find the best matching documents for a specific search array. The search array is the filter that matches documents against the user’s interest. In this application, each document is reduced to a set of 32-bit words, where each word is a pair of 24-bit word ID and 8-bit frequency representing the occurrence of the word ID in the document. The search array consists of smaller set of word IDs and each word ID has a weight associated with it, which represents the significance of the word. The application computes a score for each document to determine its relevance to the given search array.

## Run the Application on the CPU

Navigate to the `cpu_src` directory and run the following command.

``` 
make run
```

The output is as follows.
 ```
 Total execution time of CPU         |  4112.5895 ms
 Hash function processing time       |  3660.4433 ms
 CPU post processing time            |   452.1462 ms
--------------------------------------------------------------------
```

>**NOTE:** The performance number might vary depending on the CPU machine and workload activity at that time.

The above command computes the score for 100,000 documents (1.39 GB) in this application. The execution time for this application is 4.112 seconds. The throughput is as follows:

Throughput = Total data/Total time = 1.39 GB/4.112s = 338 MB/s

## Profiling the Application

To improve the performance of the application, you need to identify bottlenecks in this application where the application is spending majority of the time.

As we can see from the execution times in the previous step, the applications spends 89% of time in calculating the hash function and 11 % of the time in computing the document score.


## Evaluating What Code is a Good Fit for the FPGA

The `runOnCPU` function can be divided into two sections:  

* Calculating the hash function (`MurmurHash2`) of the words and creating output flags.

* Computing document scores based on output flags from the above step.

Try to analyze which function would be a good fit for implementing in the FPGA.

### Hash Function (MurmurHash2) Code

Implement the hash function (`MurmurHash2`) code as follows:

```
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
```

* From the above code, you can see that the function is compute intensive and calculates hash output without any memory access.
* The compute of hash for a single word ID consists of four XOR, 3 arithmetic shifts, and two multiplication operations.
  * A shift of 1-bit in an arithmetic shift operation takes one clock cycle on the CPU.
  * The three arithmetic operations shift a total of 44-bits (in the above code,`len=3` in above code) to compute the hash which requires 44 clock cycles just to shift the bits on CPU. Due to the custom hardware architecture possible on the FPGA, shifting by an arbitrary number of bits on the FPGA can complete the operation in one clock cycle.
* FPGA also has dedicated DSP units, which perform multiplication faster than the CPU. Even though the CPU runs at 8 times higher clock frequency than the FPGA, the arithmetic shift and multiplication operations can perform faster on FPGA because of its custom hardware architecture, enabling it to perform in fewer clock cycles compared to the CPU.

Based on the above code inspection, you can see that `MurmurHash2` is a good candidate for FPGA acceleration.

### Computing Document Score Code

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

* From the above code, you can see that the compute score requires one memory access to `profile_weights`, one addition, and one multiplication operation.
* The memory accesses are random in each loop iteration, because you do not know the word ID accessed in each consecutive word of the document.
* The size of `profile_weights` array is 128 MB and is placed in the FPGA DDR. If this function is implemented on the FPGA, the accesses to this array can slow down the performance while computing the score, so you can keep this function on CPU.

 Based on above code inspection, you will only offload `MurmurHash2` function on FPGA.

## Run the Application on the FPGA

When the `MurmurHash2` function is implemented in the FPGA, the FPGA returns a byte for each input document word received, indicating if the word is present in the search array. In an optimal implementation, you can process 16 32-bit input document words in parallel because the maximum data width read from the FPGA is 512-bits, computing an output for 16 words in every clock cycle. This parallelization factor (reading 16 parallel document words and thus more compute in parallel) can be modified. In this tutorial, a parallelization factor of 8 is used. After offloading the `MumurHash2` function on FPGA, you see the following results.

```
--------------------------------------------------------------------
 Executed FPGA accelerated version  |   552.5344 ms   ( FPGA 528.744 ms )
 Executed Software-Only version     |   3864.4070 ms
--------------------------------------------------------------------
 Verification: PASS
```

You can see that the execution time of the application has increased almost by a factor of 7 by offloading the `MurmurHash2` function to the FPGA.  

## Conclusion

From your analysis, you see that the `MurmurHash2` function is a good candidate for FPGA acceleration as it is compute intensive with a lot of arithmetic shift and multiplication operations which can perform faster on the FPGA compared to the CPU.

Next, you need to improve the execution time of the `MurmurHash2` function on the FPGA by overlapping data transfers and computes between the host and FPGA, as the kernel is already optimized, and its execution time is close to theoretical number (explained in the next part). Because the kernel is already optimized, the only way to improve the execution time on the FPGA is by [overlapping data transfers and compute](./data_movement.md) between host and FPGA. To further improve the performance of application, you overlap FPGA hash function processing with CPU document score to achieve an optimal execution time.
