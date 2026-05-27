`timescale 1ns/1ps

module ip_router_tb;
    logic clk   = 0;
    logic reset = 1;
    logic       grant1;
    logic       req1;
    logic [9:0] data1;
    logic       grant2;
    logic       req2;
    logic [9:0] data2;
    logic       valid;
    logic [7:0] data_out;

    always #5 clk = ~clk;

    ip_enc_generator u_enc (
        .clk   (clk),
        .rst   (reset),
        .grant (grant1),
        .req   (req1),
        .data  (data1)
    );

    ip_dec_generator u_dec (
        .clk   (clk),
        .rst   (reset),
        .grant (grant2),
        .req   (req2),
        .data  (data2)
    );

    mini_router u_router (
        .clk      (clk),
        .reset    (reset),
        .req1     (req1),
        .data1    (data1),
        .req2     (req2),
        .data2    (data2),
        .grant1   (grant1),
        .grant2   (grant2),
        .valid    (valid),
        .data_out (data_out)
    );

    initial begin
        repeat(2) @(posedge clk);
        reset = 0;
        repeat(50) @(posedge clk);
        $finish;
    end

    initial begin
        $dumpfile("ip_router_tb.vcd");
        $dumpvars(0, ip_router_tb);
        $monitor("req1 = %b data1_pri = %0d data1_byte = 0x%02h grant1 = %b | req2 = %b data2_pri = %0d data2_byte = 0x%02h grant2 = %b | valid = %b data_out = 0x%02h",
            req1, data1[9:8], data1[7:0], grant1,
            req2, data2[9:8], data2[7:0], grant2,
            valid, data_out);
    end

endmodule