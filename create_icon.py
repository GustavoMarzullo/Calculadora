#!/usr/bin/env python3
"""
Script para criar um ícone personalizado para a Calculadora de Sistemas Lineares
Requer: pip install Pillow
"""

try:
    from PIL import Image, ImageDraw, ImageFont
    import os
except ImportError:
    print("ERRO: Pillow não está instalado.")
    print("Execute: pip install Pillow")
    exit(1)

def create_calculator_icon():
    """Cria um ícone elegante para a calculadora"""
    
    # Configurações do ícone
    size = 256
    bg_color = (250, 248, 245)  # Off-white
    panel_color = (245, 240, 235)  # Bege claro
    accent_color = (210, 200, 185)  # Bege médio
    text_color = (60, 55, 50)  # Marrom escuro
    
    # Criar imagem
    img = Image.new('RGBA', (size, size), bg_color + (255,))
    draw = ImageDraw.Draw(img)
    
    # Desenhar fundo com gradiente sutil
    margin = 20
    
    # Painel principal (calculadora)
    panel_rect = [margin, margin, size-margin, size-margin]
    draw.rounded_rectangle(panel_rect, radius=20, fill=panel_color + (255,), outline=accent_color + (255,), width=3)
    
    # Tela da calculadora (parte superior)
    screen_margin = margin + 15
    screen_height = 60
    screen_rect = [screen_margin, screen_margin, size-screen_margin, screen_margin + screen_height]
    draw.rounded_rectangle(screen_rect, radius=8, fill=(255, 255, 255, 255), outline=accent_color + (255,), width=2)
    
    # Texto na tela (equação exemplo)
    try:
        # Tentar usar uma fonte do sistema
        font_size = 16
        font = ImageFont.truetype("arial.ttf", font_size)
    except:
        # Fallback para fonte padrão
        font = ImageFont.load_default()
    
    equation_text = "2x + 3y = 7"
    text_bbox = draw.textbbox((0, 0), equation_text, font=font)
    text_width = text_bbox[2] - text_bbox[0]
    text_height = text_bbox[3] - text_bbox[1]
    text_x = screen_margin + (screen_rect[2] - screen_margin - text_width) // 2
    text_y = screen_margin + (screen_height - text_height) // 2
    draw.text((text_x, text_y), equation_text, fill=text_color + (255,), font=font)
    
    # Botões da calculadora (grid 4x4)
    button_start_y = screen_margin + screen_height + 20
    button_size = 35
    button_spacing = 10
    buttons_per_row = 4
    
    # Símbolos dos botões
    button_symbols = [
        ['x₁', 'x₂', 'x₃', '='],
        ['1', '2', '3', '+'],
        ['4', '5', '6', '-'],
        ['7', '8', '9', '×']
    ]
    
    for row in range(4):
        for col in range(4):
            if row < len(button_symbols) and col < len(button_symbols[row]):
                x = screen_margin + col * (button_size + button_spacing)
                y = button_start_y + row * (button_size + button_spacing)
                
                # Desenhar botão
                button_rect = [x, y, x + button_size, y + button_size]
                draw.rounded_rectangle(button_rect, radius=6, fill=(255, 255, 255, 255), outline=accent_color + (255,), width=1)
                
                # Texto do botão
                symbol = button_symbols[row][col]
                try:
                    button_font = ImageFont.truetype("arial.ttf", 12)
                except:
                    button_font = ImageFont.load_default()
                
                symbol_bbox = draw.textbbox((0, 0), symbol, font=button_font)
                symbol_width = symbol_bbox[2] - symbol_bbox[0]
                symbol_height = symbol_bbox[3] - symbol_bbox[1]
                symbol_x = x + (button_size - symbol_width) // 2
                symbol_y = y + (button_size - symbol_height) // 2
                draw.text((symbol_x, symbol_y), symbol, fill=text_color + (255,), font=button_font)
    
    # Adicionar símbolo matemático no canto
    try:
        math_font = ImageFont.truetype("arial.ttf", 24)
    except:
        math_font = ImageFont.load_default()
    
    math_symbol = "∑"
    math_x = size - margin - 30
    math_y = size - margin - 30
    draw.text((math_x, math_y), math_symbol, fill=accent_color + (255,), font=math_font)
    
    # Salvar em diferentes tamanhos para o ICO
    sizes = [16, 32, 48, 64, 128, 256]
    images = []
    
    for ico_size in sizes:
        resized = img.resize((ico_size, ico_size), Image.Resampling.LANCZOS)
        images.append(resized)
    
    # Salvar como ICO
    img.save('calculator.ico', format='ICO', sizes=[(s, s) for s in sizes])
    print(f"Ícone criado com sucesso: calculator.ico")
    
    # Também salvar como PNG para visualização
    img.save('calculator_icon_preview.png', format='PNG')
    print(f"Preview criado: calculator_icon_preview.png")

if __name__ == "__main__":
    print("Criando ícone personalizado para a Calculadora de Sistemas Lineares...")
    create_calculator_icon()
    print("Concluído!")


