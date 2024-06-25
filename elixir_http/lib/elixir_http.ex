defmodule ElixirHttp do
  @moduledoc """
  Documentation for `ElixirHttp`.
  """

  @doc """
  Hello world.

  ## Examples

      iex> ElixirHttp.hello()
      :world

  """
  require Logger

  def accept(port) do
    # The options below mean:
    #
    # 1. `:binary` - receives data as binaries (instead of lists)
    # 2. `packet: :line` - receives data line by line
    # 3. `active: false` - blocks on `:gen_tcp.recv/2` until data is available
    # 4. `reuseaddr: true` - allows us to reuse the address if the listener crashes
    #
    {:ok, socket} =
      :gen_tcp.listen(port, [:binary, packet: :line, active: false, reuseaddr: true])

    Logger.info("Accepting connections on port #{port}")
    loop_acceptor(socket)
  end

  defp loop_acceptor(socket) do
    {:ok, client} = :gen_tcp.accept(socket)
    {:ok, pid} = Task.Supervisor.start_child(ElixirHttp.TaskSupervisor, fn -> serve(client) end)
    IO.puts(inspect(pid))
    :ok = :gen_tcp.controlling_process(client, pid)
    loop_acceptor(socket)
  end

  defp parse_http(line, state) do
    case state do
      0 -> String.split(line, " ")
    end
  end

  defp serve(socket) do
    case :gen_tcp.recv(socket, 0) do
      {:ok, data} ->
        parse_http(data, 0)
        contents = "test"

        :gen_tcp.send(
          socket,
          "HTTP/1.1 200 OK\r\nContent-Length: #{String.length(contents)}\r\n\r\n#{contents}"
        )

      {:error, :enotconn} ->
        Logger.info("Socket #{inspect(self())} left quickly, goodbye :(")
        exit(:shutdown)

      {:error, :closed} ->
        Logger.info("Socket #{inspect(self())} left, goodbye :)")
        exit(:shutdown)
    end

    serve(socket)
  end
end
