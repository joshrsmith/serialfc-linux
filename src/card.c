#include <linux/slab.h>
#include <linux/pci.h> /* struct pci_dev */

#include "card.h"
#include "port.h"
#include "utils.h"
#include "serialfc.h"

struct pciserial_board fc_2_pci_board = {
	.flags = FL_BASE0,
	.num_ports = 2,
	.base_baud = 1152000,
	.uart_offset = 0x200,
};

struct pciserial_board fc_4_pci_board = {
	.flags = FL_BASE0,
	.num_ports = 4,
	.base_baud = 1152000,
	.uart_offset = 0x200,
};

struct pciserial_board fc_8_pci_board = {
	.flags = FL_BASE0,
	.num_ports = 8,
	.base_baud = 1152000,
	.uart_offset = 0x200,
};

struct pciserial_board fc_2_pcie_board = {
	.flags = FL_BASE0,
	.num_ports = 2,
	.base_baud = 7812500,
	.uart_offset = 0x400,
	.reg_shift = 0,
	.first_offset = 0,
};

struct pciserial_board fc_4_pcie_board = {
	.flags = FL_BASE0,
	.num_ports = 4,
	.base_baud = 7812500,
	.uart_offset = 0x400,
	.reg_shift = 0,
	.first_offset = 0,
};

struct pciserial_board fc_8_pcie_board = {
	.flags = FL_BASE0,
	.num_ports = 8,
	.base_baud = 7812500,
	.uart_offset = 0x400,
	.reg_shift = 0,
	.first_offset = 0,
};

struct fc_card *fc_card_new(struct pci_dev *pdev)
{
	struct fc_card *card = 0;
	struct fc_port *port_iter = 0;
	struct pciserial_board *board = 0;
	unsigned i = 0;

	card = kmalloc(sizeof(*card), GFP_KERNEL);

	return_val_if_untrue(card != NULL, 0);

	INIT_LIST_HEAD(&card->list);
	INIT_LIST_HEAD(&card->ports);

	card->pci_dev = pdev;

	switch (pdev->device) {
	case FC_422_2_PCI_335_ID:
		board = &fc_2_pci_board;
		break;

	case FC_422_4_PCI_335_ID:
	case FC_232_4_PCI_335_ID:
		board = &fc_4_pci_board;
		break;

	case FC_232_8_PCI_335_ID:
		board = &fc_8_pci_board;
		break;

	case FC_422_4_PCIe_ID:
		board = &fc_4_pcie_board;
		break;

	case FC_422_8_PCIe_ID:
		board = &fc_8_pcie_board;
		break;
	}

	card->serial_priv = pciserial_init_ports(pdev, board);

	if (IS_ERR(card->serial_priv)) {
		dev_err(&card->pci_dev->dev, "pciserial_init_ports failed\n");
		return 0;
	}

	pci_set_drvdata(pdev, card->serial_priv);

	card->addr = pci_iomap(card->pci_dev, 0, 0);

	if (card->addr == NULL) {
		dev_err(&card->pci_dev->dev, "pci_iomap failed\n");
		return 0;
	}

	/* There are two ports per card. */
	for (i = 0; i < board->num_ports; i++) {
		port_iter = fc_port_new(card, i, card->addr + (board->uart_offset * i));

		if (port_iter)
			list_add_tail(&port_iter->list, &card->ports);
	}

	return card;
}

void fc_card_delete(struct fc_card *card)
{
	struct list_head *current_node = 0;
	struct list_head *temp_node = 0;

	return_if_untrue(card);

	list_for_each_safe(current_node, temp_node, &card->ports) {
		struct fc_port *current_port = 0;

		current_port = list_entry(current_node, struct fc_port, list);

		list_del(current_node);
		fc_port_delete(current_port);
	}

	pciserial_remove_ports(card->serial_priv);
	pci_set_drvdata(card->pci_dev, NULL);

	kfree(card);
}