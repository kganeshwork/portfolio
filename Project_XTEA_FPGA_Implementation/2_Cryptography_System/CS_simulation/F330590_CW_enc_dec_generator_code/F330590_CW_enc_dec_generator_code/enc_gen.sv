`timescale 1ns/1ps

module ip_enc_generator (
    input  logic       clk,
    input  logic       rst,
    input  logic       grant,
    output logic [9:0] data,  
    output logic       req
);
    logic [7:0] lfsr;

    always_ff @(posedge clk or posedge rst) begin
        if (rst) lfsr <= 8'hAC;
        else     lfsr <= {lfsr[6:0], lfsr[7] ^ lfsr[5] ^ lfsr[4] ^ lfsr[3]};
    end

    assign data = {lfsr[1:0], lfsr[7:0]};
    assign req = lfsr[3];
    
endmodule