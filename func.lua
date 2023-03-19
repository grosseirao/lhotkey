target("Terraria.bin.x86_64")
State = false
function KEY_F9()
    State = not State
    input_key("BTN_LEFT", State and 1 or 0)
    print("O resultado é: F9\n")
end
