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

    // Override signals for controlled testing
    logic       req1_ovr,  req2_ovr;
    logic [9:0] data1_ovr, data2_ovr;
    logic       use_override = 1;
    logic       req1_gen,  req2_gen;
    logic [9:0] data1_gen, data2_gen;

    assign req1  = use_override ? req1_ovr  : req1_gen;
    assign req2  = use_override ? req2_ovr  : req2_gen;
    assign data1 = use_override ? data1_ovr : data1_gen;
    assign data2 = use_override ? data2_ovr : data2_gen;

    // Clock generation
    always #5 clk = ~clk;

    // IP Enc generator 
    ip_enc_generator u_enc (
        .clk   (clk),
        .rst   (reset),
        .grant (grant1),
        .req   (req1_gen),
        .data  (data1_gen)
    );

    // IP Dec generator 
    ip_dec_generator u_dec (
        .clk   (clk),
        .rst   (reset),
        .grant (grant2),
        .req   (req2_gen),
        .data  (data2_gen)
    );

    // Mini Router
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

    // Stimulus
    initial begin
        reset = 1;
        req1_ovr = 0; req2_ovr = 0;
        data1_ovr = 0; data2_ovr = 0;
        repeat(2) @(posedge clk);
        reset = 0;
        @(posedge clk);

        // Test 1: neither req is high
        req1_ovr = 0; req2_ovr = 0;
        data1_ovr = {"00", 8'hAA}; data2_ovr = {"00", 8'hBB};
        @(posedge clk); #1;
        assert (valid == 0 && grant1 == 0 && grant2 == 0)
            else $error("Test 1 FAIL");

        //  Test 2: only req1 is high 
        req1_ovr = 1; req2_ovr = 0;
        data1_ovr = {"10", 8'hA5}; data2_ovr = {"00", 8'h00};
        @(posedge clk); #1;
        assert (data_out == 8'hA5 && grant1 == 1 && grant2 == 0 && valid == 1)
            else $error("Test 2 FAIL");

        // Test 3: only req2 is high
        req1_ovr = 0; req2_ovr = 1;
        data1_ovr = {"00", 8'h00}; data2_ovr = {"01", 8'hB6};
        @(posedge clk); #1;
        assert (data_out == 8'hB6 && grant1 == 0 && grant2 == 1 && valid == 1)
            else $error("Test 3 FAIL");

        // Test 4: both req are high, link1 has higher priority
        req1_ovr = 1; req2_ovr = 1;
        data1_ovr = {"11", 8'hC1}; data2_ovr = {"01", 8'hD1};
        @(posedge clk); #1;
        assert (data_out == 8'hC1 && grant1 == 1 && grant2 == 0 && valid == 1)
            else $error("Test 4 FAIL");

        // Test 5: both req are high, link2 has higher priority 
        req1_ovr = 1; req2_ovr = 1;
        data1_ovr = {"01", 8'hC2}; data2_ovr = {"11", 8'hD2};
        @(posedge clk); #1;
        assert (data_out == 8'hD2 && grant1 == 0 && grant2 == 1 && valid == 1)
            else $error("Test 5 FAIL");
         
        req1_ovr = 0; req2_ovr = 0;
        data1_ovr = {"00", 8'h00}; data2_ovr = {"00", 8'h00};
        @(posedge clk);

        @(posedge clk); 

        // Test 6: round robin 1st conflict - link1 should win
        req1_ovr = 1; req2_ovr = 1;
        data1_ovr = {"01", 8'hE1}; data2_ovr = {"01", 8'hF1};
        @(posedge clk); #1;
        assert (data_out == 8'hE1 && grant1 == 1 && grant2 == 0 && valid == 1)
            else $error("Test 6 FAIL");

        // Test 7: round robin 2nd conflict - link2 should win 
        req1_ovr = 1; req2_ovr = 1;
        data1_ovr = {"01", 8'hE2}; data2_ovr = {"01", 8'hF2};
        @(posedge clk); #1;
        assert (data_out == 8'hF2 && grant1 == 0 && grant2 == 1 && valid == 1)
            else $error("Test 7 FAIL");

        // Test 8: round robin 3rd conflict - link1 should win again 
        req1_ovr = 1; req2_ovr = 1;
        data1_ovr = {"01", 8'hE3}; data2_ovr = {"01", 8'hF3};
        @(posedge clk); #1;
        assert (data_out == 8'hE3 && grant1 == 1 && grant2 == 0 && valid == 1)
            else $error("Test 8 FAIL");

        $display("Controlled tests complete - Switch to Enc and Dec being generated");

        // Switch from controlled testing to data from Enc and Dec generators
        use_override = 0;
        repeat(10) @(posedge clk);

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