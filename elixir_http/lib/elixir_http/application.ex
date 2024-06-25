defmodule ElixirHttp.Application do
  # See https://hexdocs.pm/elixir/Application.html
  # for more information on OTP Applications
  @moduledoc false

  use Application

  @impl true
  def start(_type, _args) do
    port = String.to_integer(System.get_env("PORT") || "4040")

    children = [
      # Starts a worker by calling: ElixirHttp.Worker.start_link(arg)
      # {ElixirHttp.Worker, arg}
      {Task.Supervisor, name: ElixirHttp.TaskSupervisor},
      Supervisor.child_spec({Task, fn -> ElixirHttp.accept(port) end}, restart: :permanent)
    ]

    # See https://hexdocs.pm/elixir/Supervisor.html
    # for other strategies and supported options
    opts = [strategy: :one_for_one, name: ElixirHttp.Supervisor]
    Supervisor.start_link(children, opts)
  end
end
