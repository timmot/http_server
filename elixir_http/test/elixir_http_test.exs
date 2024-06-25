defmodule ElixirHttpTest do
  use ExUnit.Case
  doctest ElixirHttp

  test "greets the world" do
    assert ElixirHttp.hello() == :world
  end
end
