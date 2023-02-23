# Embedded Systems Coursework 2
## Generating audio samples with a double buffer (WIP)

An audio synthesiser needs to generate a new sample every $1/f_s$ seconds.
The sample frequency is much higher than an RTOS scheduler tick rate, so there remain two options for updating the DAC output:

1. Use an interrupt which is triggered off a timer with a period of $1/f_s$. This is the method used in the lab notes
2. Set up a DMA module to automatically copy a new sample from memory to the DAC at the required interval

Generating an audio sample can be computationally expensive, especially if convolution is used to implement a filter, or if several waveforms are being summed together.
Doing this computation in an ISR can cause problems, such as:

- Need to set interrupt priorities carefully so that other interrupts complete on time.
  There can be a conflict with the RTOS, particularly if you need a higher priority than the interrupt used for the RTOS tick.
- Unavailability of blocking synchronisation functions (e.g. lock a mutex) makes synchronisation more difficult.
  The result is often a proliferation of critical sections in thread functions, which should be avoided
  
A common solution is to calculate samples in batches in a lower-priority thread and use the interrupt just to copy the sample to the DAC.
This is also the best approach when using the DMA, since the DMA can auto-increment its read pointer and copy successive bytes from memory.

Whether the copy to DAC is done by interrupt or DMA, the array of samples is shared between two tasks: sample generation and copy to DAC.
That means it requires synchronisation.

### Double Buffer

A common method of synchronising this kind of transfer is to use a double buffer, or ping-pong buffer.
A double buffer is simpler and more efficient than a queue, but it can be written by only one task and read by only one task.

The principle is to split an array into two.
The writing task will access one half and the reading task will access the other.
When both tasks have finished accessing half the buffer, they will swap pointers and access the other half.

Synchronisation is achieved by ensuring that the pointer swap happens atomically, so the two tasks never access the same half of the buffer.

### Coursework Implementation with ISR

Create an array that will hold the samples, with a pointer to the half-way point and a flag that will define which half is being written:

```c++
uint32_t sampleBuffer0[SAMPLE_BUFFER_SIZE];
uint32_t sampleBuffer1[SAMPLE_BUFFER_SIZE];
volatile bool writeBuffer1 = false;
```

The ISR will copy one sample to the DAC and increment a write counter.
The flag is used to determine which buffer to read from.
When the pointer reaches `SAMPLE_BUFFER_SIZE`, the counter will reset to zero and the pointers will swap:

```c++
static uint32_t readCtr = 0;

if (readCtr == SAMPLE_BUFFER_SIZE) {
	readCtr = 0;
	writeBuffer1 = !writeBuffer1;
	xSemaphoreGiveFromISR(sampleBufferSemaphore, NULL);
	}
	
if (writeBuffer1)
	analogWrite(OUTR_PIN, sampleBuffer0[readCtr++]);
else
	analogWrite(OUTR_PIN, sampleBuffer1[readCtr++]);
```

A sempaphore is given by the ISR when the buffer swaps.
That semaphore will be used to block the generator thread until the next buffer segment is ready for a new batch of samples:

```c++
xSemaphoreTake(sampleBufferSemaphore, portMAX_DELAY);
for (uint32_t writeCtr = 0; writeCtr < SAMPLE_BUFFER_SIZE; writeCtr++) {
	uint32_t Vout = … //Calculate one sample
	if (writeBuffer1)
		sampleBuffer1[writeCtr] = Vout;
	else
		sampleBuffer0[writeCtr] = Vout;
```
	
### Buffer size
The size of the sample buffer affects the priority of the sample generation thread and the latency between generating a sample and it appearing on the output.
One batch of samples must be generated for every $n$ samples, where $n$ is the size of each buffer.
If samples are consumed at $f_s$, then sample generation has an initiation interval of $n/f_s$ seconds, and the priority of the thread must be set accordingly.

Meanwhile, the latency $L$ between generation and output depends on the time it takes to generate a batch of samples.
$L \gt n/f_s$, since every sample must wait for a pointer swap until it is output.
The maximum latency depends on the rate of sample generation, which will be faster than sample consumption until a batch is complete and the generator thread waits for a pointer swap.
If sample generation is very fast, then the last sample in a batch could be generated soon after a pointer swap and sent to the output at the end of the next cycle.
So $n/f_s\lt L \lt 2n/f_s$ seconds.
If $L$ is too large then there could be a noticable delay between a user input and the resulting sound.

### Limitations and assumptions
The double bufffer implementation here is lightweight but it makes some assumptions:
1.	The ISR makes several access to global memory: the sample buffer, the `writeBuffer1` flag and the semaphore.
	There is no attempt to keep them synchronised and we assume that there is no higher-priority task that could prempt this interrupt and access these variables.
	For example, there is no possibility of the write thread acting on the change to `writeBuffer1` before the semaphore is given in the next line.
2.	The `writeBuffer1` flag is atomic, but the generator thread uses it non-atomically to decide which buffer to write to.
	There is a possibility of the flag changing between the test and the write to a sample buffer, so the flag doesn't fully protect against simultaneous access to the same buffer.
	This could be resolved by placing the flag test and the buffer write in a critical section.
	
	However, we do not expect the flag to change during a batch of sample generation because the flag changes are synchroised with the initiation intervals of the generator task.
	If it does change, that means generation cannot keep up with the sample rate — any synchronisation failure here would be a side-effect of a deadline miss.
	The code could be improved by testing for this condition and indicating an exception in some way.
3. 	The code uses C arrays, which have no built-in protection against out-of-bounds access.
	We assume there are no bugs that would result in accessing data outside an array.
	
### Double buffering with DMA
The STM32 DMA module is designed for use with double buffering.
To adapt the code from above:
1.	Make the sample buffers contiguous in memory:
	
	```c++
	uint32_t sampleBuffer0[SAMPLE_BUFFER_SIZE*2];
	uint32_t sampleBuffer1[] = sampleBuffer0 + SAMPLE_BUFFER_SIZE;
	```
2.	Set the DMA to copy samples from `sampleBuffer0` to the DAC at $f_s$.
	The read pointer should be set to auto-increment.
3.	Enable interrupts when the DMA read is complete and when it is half-complete.
	Write an ISR that will trigger the buffer swap and release the generator thread.

