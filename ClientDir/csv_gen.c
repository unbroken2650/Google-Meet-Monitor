#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("Starting Program...");
    system("touch output.pcap");
    system("chmod 777 output.pcap");
    system("sudo tshark -a duration:10 -i 1 -f \"udp\" -w output.pcap");
    sleep(10); // Delay for 10 seconds
    system("sudo tshark -r output.pcap -T fields -E header=y -E separator=, -E quote=d -e frame.number -e frame.time_epoch -e ip.src -e ip.dst -e udp.srcport -e udp.dstport -e data.data -e rtcp.pt -e rtcp.ssrc.fraction > output.csv");
    return 0;
}