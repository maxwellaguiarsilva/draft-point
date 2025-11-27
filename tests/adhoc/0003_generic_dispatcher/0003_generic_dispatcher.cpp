/*
 * Copyright (C) 2025 Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/* 
 * File:   0003_generic_dispatcher.cpp
 * Author: Maxwell Aguiar Silva <maxwellaguiarsilva@gmail.com>
 * 
 * Created on 2025-11-26 21:28:59
 */


#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <memory>


template <typename listener_interface_type>
class event_dispatcher {
public:
    void add_listener(std::shared_ptr<listener_interface_type> listener_instance) {
        registered_listeners.push_back(listener_instance);
    }

    void remove_listener(std::shared_ptr<listener_interface_type> listener_instance) {
        registered_listeners.erase(
            std::remove_if(registered_listeners.begin(), registered_listeners.end(),
                [&](const std::weak_ptr<listener_interface_type>& wp) {
                    auto sp = wp.lock();
                    return !sp || sp == listener_instance; 
                }),
            registered_listeners.end()
        );
    }

    // --- CORREÇÃO APLICADA AQUI ---
    // Usamos MethodArgs para deduzir a assinatura do ponteiro de função
    // E CallArgs para deduzir os argumentos passados no broadcast
    template <typename... MethodArgs, typename... CallArgs>
    void broadcast(void (listener_interface_type::*member_function_pointer)(MethodArgs...), CallArgs&&... arguments) {
        auto it = registered_listeners.begin();
        while (it != registered_listeners.end()) {
            if (auto strong_ptr = it->lock()) {
                (strong_ptr.get()->*member_function_pointer)(std::forward<CallArgs>(arguments)...);
                ++it;
            } else {
                it = registered_listeners.erase(it);
            }
        }
    }

private:
    std::vector<std::weak_ptr<listener_interface_type>> registered_listeners;
};

class button_listener {
public:
    virtual ~button_listener() = default;
    // Note que a interface pede const std::string&
    virtual void on_clicked(const std::string& button_name) = 0;
    virtual void on_hover(int duration) = 0;
};

class concrete_button_logger : public button_listener {
public:
    void on_clicked(const std::string& button_name) override {
        std::cout << "Button clicked: " << button_name << std::endl;
    }
    void on_hover(int duration) override {
        std::cout << "Hover: " << duration << std::endl;
    }
};

int main() {
    auto my_logger = std::make_shared<concrete_button_logger>();
    event_dispatcher<button_listener> notifier;
    
    notifier.add_listener(my_logger);

    // Agora isso funciona!
    // O compilador deduz MethodArgs = {const std::string&}
    // E CallArgs = {std::string}
    // E sabe que pode passar um para o outro.
    notifier.broadcast(&button_listener::on_clicked, std::string("btn_start"));
    
    // Isso também funciona (int para int)
    notifier.broadcast(&button_listener::on_hover, 100);

    return 0;
}
