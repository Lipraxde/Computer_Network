set ns [new Simulator]
set nf [open hw2.nam w]
set rf0 [open hw2_0.tr w]
set rf1 [open hw2_1.tr w]
set rf2 [open hw2_2.tr w]
set rf3 [open hw2_3.tr w]
set rf4 [open hw2_4.tr w]

$ns namtrace-all $nf

$ns color 0 Red
$ns color 1 Orange
$ns color 2 Yellow
$ns color 3 Green
$ns color 4 Blue

set reciver [$ns node]
for {set i 0} {$i<5} {incr i} {
    set senders($i) [$ns node]
    set routers($i) [$ns node]

    #Create links between the nodes
    $ns duplex-link $senders($i) $routers($i) 5.0Mb 20ms DropTail
    $ns duplex-link-op $senders($i) $routers($i) orient down

    #Setup TCPs connection
    set tcp($i) [new Agent/TCP]
    $tcp($i) set class_ 2
    $ns attach-agent $senders($i) $tcp($i)
    $tcp($i) set fid_ $i

    set sink($i) [new Agent/TCPSink]
    $ns attach-agent $reciver $sink($i)

    $ns connect $tcp($i) $sink($i)

    #Setup FTPs over TCPs connection
    set ftp($i) [new Application/FTP]
    $ftp($i) attach-agent $tcp($i)
    $ftp($i) set type_ FTP
}

$ns duplex-link $routers(0) $routers(1) 0.5Mb 100ms DropTail
$ns duplex-link $routers(1) $routers(2) 0.5Mb 100ms DropTail
$ns duplex-link $routers(2) $routers(3) 0.5Mb 100ms DropTail
$ns duplex-link $routers(3) $routers(4) 0.5Mb 100ms DropTail
$ns duplex-link $routers(4) $reciver 0.5Mb 100ms DropTail

#Give node position (for NAM)
$ns duplex-link-op $routers(0) $routers(1) orient right
$ns duplex-link-op $routers(1) $routers(2) orient right
$ns duplex-link-op $routers(2) $routers(3) orient right
$ns duplex-link-op $routers(3) $routers(4) orient right
$ns duplex-link-op $routers(4) $reciver orient right

#Monitor the queue for link (n2-n3). (for NAM)
$ns duplex-link-op $routers(0) $routers(1) queuePos 1.5
$ns duplex-link-op $routers(1) $routers(2) queuePos 1.5
$ns duplex-link-op $routers(2) $routers(3) queuePos 1.5
$ns duplex-link-op $routers(3) $routers(4) queuePos 1.5
$ns duplex-link-op $routers(4) $reciver queuePos 1.5

#$ns queue-limit $r0 $r1 10
#$ns queue-limit $r1 $r2 10
#$ns queue-limit $r2 $r3 10
#$ns queue-limit $r3 $r4 10
#$ns queue-limit $r4 $ne 10

#$ns at 0.0 "$n0 lable Sender"
#$ns at 0.0 "$n1 lable Sender"
#$ns at 0.0 "$n2 lable Sender"
#$ns at 0.0 "$n3 lable Sender"
#$ns at 0.0 "$n4 lable Sender"
#$ns at 0.0 "$ne lable Sink"
# 
#$ns at 0.0 "$r0 lable Router"
#$ns at 0.0 "$r1 lable Router"
#$ns at 0.0 "$r2 lable Router"
#$ns at 0.0 "$r3 lable Router"
#$ns at 0.0 "$r4 lable Router"

$ns at 0.0 "record"
set i 0
for {set i 0} {$i<5} {incr i} {
    $ns at 0.1 "$ftp($i) start"
    $ns at 10 "$ftp($i) stop"
}
$ns at 15 "finish"

set time_move 0.10
set sliding_window 0.50
set alpha 0.10
set beta [expr 1/($alpha)]
set old_vlaue(0) 0
set old_vlaue(1) 0
set old_vlaue(2) 0
set old_vlaue(3) 0
set old_vlaue(4) 0

proc record {} {
    global sink rf0 rf1 rf2 rf3 rf4 time_move sliding_window alpha beta old_vlaue
    #Get an instance of the simulator
    set ns [Simulator instance]
    #Set the time after which the procedure should be called again
    set time $time_move
    #How many bytes have been received by the traffic sinks?
    set old_vlaue(0) [expr $old_vlaue(0)*(1-$alpha)+[$sink(0) set bytes_]]
    set old_vlaue(1) [expr $old_vlaue(1)*(1-$alpha)+[$sink(1) set bytes_]]
    set old_vlaue(2) [expr $old_vlaue(2)*(1-$alpha)+[$sink(2) set bytes_]]
    set old_vlaue(3) [expr $old_vlaue(3)*(1-$alpha)+[$sink(3) set bytes_]]
    set old_vlaue(4) [expr $old_vlaue(4)*(1-$alpha)+[$sink(4) set bytes_]]
    set bw0 [expr $old_vlaue(0)/$beta]
    set bw1 [expr $old_vlaue(1)/$beta]
    set bw2 [expr $old_vlaue(2)/$beta]
    set bw3 [expr $old_vlaue(3)/$beta]
    set bw4 [expr $old_vlaue(4)/$beta]
    #Get the current time
    set now [$ns now]
    #Calculate the bandwidth (in MBit/s) and write it to the files1
    puts $rf0 "$now [expr $bw0/$time*8/1000000]"
    puts $rf1 "$now [expr $bw1/$time*8/1000000]"
    puts $rf2 "$now [expr $bw2/$time*8/1000000]"
    puts $rf3 "$now [expr $bw3/$time*8/1000000]"
    puts $rf4 "$now [expr $bw4/$time*8/1000000]"
    #Reset the bytes_ values on the traffic sinks
    $sink(0) set bytes_ 0
    $sink(1) set bytes_ 0
    $sink(2) set bytes_ 0
    $sink(3) set bytes_ 0
    $sink(4) set bytes_ 0
    #Re-schedule the procedure
    $ns at [expr $now+$time] "record"
}


proc finish {} {
    global ns nf rf0 rf1 rf2 rf3 rf4
    $ns flush-trace
    close $nf 
    close $rf0
    close $rf1
    close $rf2
    close $rf3
    close $rf4

    catch { exec nam hw2.nam & }
    catch { exec xgraph hw2_0.tr hw2_1.tr hw2_2.tr hw2_3.tr hw2_4.tr -geometry 800x400 & }
    
    catch { exec gnuplot plot.gp & }
    exit 0
}

$ns run
