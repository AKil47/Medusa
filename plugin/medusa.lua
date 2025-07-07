if vim.g.loaded_medusa then
    return
end
vim.g.loaded_medusa = 1

require('medusa').setup()