require 'test/unit'
require 'mtcp'

Thread.abort_on_exception = true

class Test_MTCP < Test::Unit::TestCase
  def setup
    @server = MTCP::Server.new("localhost", 0)
      # windows needs the "localhost"
    @port = @server.addr[1]
    @host = @server.addr[3]
  end
  
  def test_everything
    server_received = []
    client_received = []
    
    messages = [
      "yowp",
      "bwang",
      "foo bar",
      "whupple snout"
    ]
    
    @server_thread = Thread.new do
      session = @server.accept
      server_received << session.recv_message
      server_received << session.recv_message
      session.send_message messages.shift
      session.send_message messages.shift
    end
    
    Thread.pass # let the server start
    
    @client_thread = Thread.new do
      @client = MTCP::Socket.new(@host, @port)
      @client.send_message messages.shift
      @client.send_message messages.shift
      client_received << @client.recv_message
      client_received << @client.recv_message
    end
    
    @server_thread.join
    @client_thread.join
    
    assert_equal(["yowp", "bwang"], server_received)
    assert_equal(["foo bar", "whupple snout"], client_received)
  end
end


puts "-------------------------------------"
